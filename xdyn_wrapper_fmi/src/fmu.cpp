// xdyn as an FMI 2.0 Co-Simulation FMU.
//
// Each instance owns a Sim and integrates it from t to t + h at each fmi2DoStep, with xdyn's
// own solver and step, as the xdyn executable does over a whole simulation: the history the
// force models read (radiation damping) goes on from one step to the next. reset_state imposes
// another state at the start of the next step: a master may teleport a vessel or switch it to
// another model, and FMI 2.0 Co-Simulation has no other way to set a state (fmi2SetFMUstate is
// opaque).

#ifndef _GNU_SOURCE
#define _GNU_SOURCE // dladdr
#endif

#include "model.hpp"
#include "xdyn_fmu.h"

#include "xdyn/external_data_structures/YamlState.hpp"
#include "xdyn/observers_and_api/CoSimulationObserver.hpp"
#include "xdyn/observers_and_api/ConfBuilder.hpp"
#include "xdyn/observers_and_api/simulator_api.hpp"

#include <ssc/solver/steppers.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <memory>
#include <sstream>

#ifdef __linux__
#include <dlfcn.h>
#endif

using namespace xdyn_fmu;

namespace
{
#ifdef __linux__
    // A dependency in the closure registers an exit handler when the library is loaded. An
    // importer that unloads the library (FMPy does, when it frees the instance) leaves that
    // handler pointing at unmapped code, and the process crashes when it exits. Reopening the
    // library RTLD_NODELETE keeps it mapped until then.
    __attribute__((constructor)) void stay_loaded()
    {
        Dl_info info;
        if (dladdr(reinterpret_cast<void*>(&stay_loaded), &info) and info.dli_fname)
            dlopen(info.dli_fname, RTLD_NOW | RTLD_NOLOAD | RTLD_NODELETE);
    }
#endif

    std::string read_file(const std::string& path)
    {
        std::ifstream in(path);
        if (not(in))
            throw std::runtime_error("cannot read " + path);
        std::stringstream s;
        s << in.rdbuf();
        return s.str();
    }

    // "file:///a/b" -> "/a/b". Only %20 is decoded: the importers we know pass plain paths.
    std::string uri_to_path(std::string uri)
    {
        if (uri.rfind("file://", 0) == 0)
            uri = uri.substr(7);
        if (uri.rfind("localhost/", 0) == 0)
            uri = uri.substr(9);
#ifdef _WIN32
        // file:///C:/x -> /C:/x
        if (uri.size() > 2 and uri[0] == '/' and uri[2] == ':')
            uri = uri.substr(1);
#endif
        for (std::string::size_type p; (p = uri.find("%20")) != std::string::npos;)
            uri.replace(p, 3, " ");
        while (uri.size() > 1 and uri.back() == '/')
            uri.pop_back();
        return uri;
    }

    // resources/parameters.txt, written by make_fmu.py: "dt <value>" and "solver <name>"
    // lines, the defaults of the xdyn_dt and solver parameters.
    void read_parameters(const std::string& path, double& dt, std::string& solver)
    {
        std::ifstream in(path);
        std::string key;
        while (in >> key)
        {
            if (key == "dt")
                in >> dt;
            else if (key == "solver")
                in >> solver;
            else
                std::getline(in, key);
        }
    }

    struct Instance
    {
        std::string name;
        fmi2CallbackFunctions callbacks{};
        bool logging = false;
        std::string resources;
        std::string yaml;
        double dt = 0.1;
        std::string solver = "rk4";
        std::unique_ptr<ConfBuilder> sim;
        // Time of the last state the history holds: a master going back before it (a restart
        // without fmi2Reset) starts a new history, xdyn's only goes forward.
        double history_time = -std::numeric_limits<double>::infinity();
        std::vector<std::string> command_names;
        std::vector<double> commands;
        std::array<double, 13> state{};
        std::array<double, 3> euler{};
        bool reset = false;
        std::array<double, 13> reset_state{};

        void log(const fmi2Status status, const char* category, const std::string& message) const
        {
            if (callbacks.logger and (logging or status != fmi2OK))
                callbacks.logger(callbacks.componentEnvironment, name.c_str(), status, category,
                                 "%s", message.c_str());
        }

        void load_model()
        {
            const Model model = read_model(yaml, resources);
            command_names = model.commands;
            commands = model.command_values;
            state = model.state;
            reset_state = model.state;
            reset = false;
            sim.reset();
            history_time = -std::numeric_limits<double>::infinity();
        }

        // Built at the end of initialization rather than at instantiation: xdyn_dt and solver
        // are parameters, set in between.
        void build_sim()
        {
            sim.reset(with_assets_root(resources, [&] { return new ConfBuilder(yaml); }));
        }

        void step(const double t, const double h)
        {
            Sim& s = sim->sim;
            // k * h drifts from (k - 1) * h + h by an ulp or so: that is not going back.
            if (t < history_time - 1e-9 * std::max(1.0, std::abs(t)))
                s.reset_history();
            std::copy(state.begin(), state.end(), s.state.begin());
            std::map<std::string, double> values;
            for (size_t k = 0; k < commands.size(); ++k)
                values[command_names[k]] = commands[k];
            s.set_command_listener(values);
            CoSimulationObserver observer({}, s.get_bodies().at(0)->get_name());
            ssc::solver::Scheduler scheduler(t, t + h, dt);
            if (solver == "euler")
                simulate<ssc::solver::EulerStepper>(s, scheduler, observer);
            else if (solver == "rk4")
                simulate<ssc::solver::RK4Stepper>(s, scheduler, observer);
            else
                simulate<ssc::solver::RKCK>(s, scheduler, observer);
            const std::vector<YamlState> out = observer.get();
            if (out.empty())
                throw std::runtime_error("xdyn returned no state");
            const YamlState& y = out.back();
            state = { y.x, y.y, y.z, y.u, y.v, y.w, y.p, y.q, y.r, y.qr, y.qi, y.qj, y.qk };
            euler = { y.phi, y.theta, y.psi };
            history_time = t + h;
        }
    };

    Instance* as(fmi2Component c) { return static_cast<Instance*>(c); }

    template <typename F> fmi2Status guarded(fmi2Component c, const char* what, F f)
    {
        if (not(c))
            return fmi2Error;
        try
        {
            return f(*as(c));
        }
        catch (const std::exception& e)
        {
            as(c)->log(fmi2Error, "logStatusError", std::string(what) + ": " + e.what());
        }
        catch (...)
        {
            as(c)->log(fmi2Error, "logStatusError", std::string(what) + ": unknown exception");
        }
        return fmi2Error;
    }

    std::string& last_error()
    {
        static std::string error;
        return error;
    }
}

extern "C"
{
    const char* xdyn_fmu_model_description(const char* yaml_path, const char* assets_root,
                                           const char* model_name, double dt, const char* solver)
    {
        static std::string xml;
        try
        {
            xml = model_description(read_model(read_file(yaml_path), assets_root), model_name, dt,
                                    solver);
            return xml.c_str();
        }
        catch (const std::exception& e)
        {
            last_error() = e.what();
            return nullptr;
        }
    }

    const char* xdyn_fmu_error(void) { return last_error().c_str(); }

    const char* fmi2GetTypesPlatform(void) { return fmi2TypesPlatform; }
    const char* fmi2GetVersion(void) { return fmi2Version; }

    fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean on, size_t, const fmi2String[])
    {
        if (not(c))
            return fmi2Error;
        as(c)->logging = on;
        return fmi2OK;
    }

    fmi2Component fmi2Instantiate(fmi2String instanceName, fmi2Type fmuType, fmi2String,
                                  fmi2String resourceLocation,
                                  const fmi2CallbackFunctions* functions, fmi2Boolean,
                                  fmi2Boolean loggingOn)
    {
        std::unique_ptr<Instance> i(new Instance);
        i->name = instanceName ? instanceName : "xdyn";
        if (functions)
            i->callbacks = *functions;
        i->logging = loggingOn;
        try
        {
            if (fmuType != fmi2CoSimulation)
                throw std::runtime_error("only Co-Simulation is supported");
            if (not(resourceLocation))
                throw std::runtime_error("no resource location");
            i->resources = uri_to_path(resourceLocation);
            i->yaml = read_file(i->resources + "/model.yml");
            read_parameters(i->resources + "/parameters.txt", i->dt, i->solver);
            i->load_model();
            return i.release();
        }
        catch (const std::exception& e)
        {
            i->log(fmi2Error, "logStatusError", std::string("fmi2Instantiate: ") + e.what());
            return nullptr;
        }
    }

    void fmi2FreeInstance(fmi2Component c) { delete as(c); }

    fmi2Status fmi2SetupExperiment(fmi2Component c, fmi2Boolean, fmi2Real, fmi2Real, fmi2Boolean,
                                   fmi2Real)
    {
        return c ? fmi2OK : fmi2Error;
    }

    fmi2Status fmi2EnterInitializationMode(fmi2Component c) { return c ? fmi2OK : fmi2Error; }

    fmi2Status fmi2ExitInitializationMode(fmi2Component c)
    {
        return guarded(c, "fmi2ExitInitializationMode", [](Instance& i) {
            i.build_sim();
            if (i.reset)
            {
                i.state = i.reset_state;
                i.reset = false;
            }
            return fmi2OK;
        });
    }

    fmi2Status fmi2Terminate(fmi2Component c) { return c ? fmi2OK : fmi2Error; }

    fmi2Status fmi2Reset(fmi2Component c)
    {
        return guarded(c, "fmi2Reset", [](Instance& i) {
            i.load_model();
            return fmi2OK;
        });
    }

    fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                           fmi2Real value[])
    {
        return guarded(c, "fmi2GetReal", [&](Instance& i) {
            for (size_t k = 0; k < nvr; ++k)
            {
                const auto r = vr[k];
                if (r >= VR_STATE and r < VR_STATE + 13)
                    value[k] = i.state[r - VR_STATE];
                else if (r >= VR_EULER and r < VR_EULER + 3)
                    value[k] = i.euler[r - VR_EULER];
                else if (r == VR_DT)
                    value[k] = i.dt;
                else if (r >= VR_RESET_STATE and r < VR_RESET_STATE + 13)
                    value[k] = i.reset_state[r - VR_RESET_STATE];
                else if (r >= VR_COMMAND and r < VR_COMMAND + i.commands.size())
                    value[k] = i.commands[r - VR_COMMAND];
                else
                    throw std::out_of_range("no Real variable with value reference "
                                            + std::to_string(r));
            }
            return fmi2OK;
        });
    }

    fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                           const fmi2Real value[])
    {
        return guarded(c, "fmi2SetReal", [&](Instance& i) {
            for (size_t k = 0; k < nvr; ++k)
            {
                const auto r = vr[k];
                if (r == VR_DT)
                {
                    if (i.sim)
                        throw std::logic_error("xdyn_dt is fixed after initialization");
                    if (not(value[k] > 0))
                        throw std::invalid_argument("xdyn_dt must be > 0");
                    i.dt = value[k];
                }
                else if (r >= VR_RESET_STATE and r < VR_RESET_STATE + 13)
                    i.reset_state[r - VR_RESET_STATE] = value[k];
                else if (r >= VR_COMMAND and r < VR_COMMAND + i.commands.size())
                    i.commands[r - VR_COMMAND] = value[k];
                else
                    throw std::out_of_range("no settable Real variable with value reference "
                                            + std::to_string(r));
            }
            return fmi2OK;
        });
    }

    fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference[], size_t nvr,
                              fmi2Integer[])
    {
        return nvr == 0 and c ? fmi2OK : fmi2Error;
    }

    fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference[], size_t nvr,
                              const fmi2Integer[])
    {
        return nvr == 0 and c ? fmi2OK : fmi2Error;
    }

    fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                              fmi2Boolean value[])
    {
        return guarded(c, "fmi2GetBoolean", [&](Instance& i) {
            for (size_t k = 0; k < nvr; ++k)
            {
                if (vr[k] != VR_RESET)
                    throw std::out_of_range("no Boolean variable with value reference "
                                            + std::to_string(vr[k]));
                value[k] = i.reset;
            }
            return fmi2OK;
        });
    }

    fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                              const fmi2Boolean value[])
    {
        return guarded(c, "fmi2SetBoolean", [&](Instance& i) {
            for (size_t k = 0; k < nvr; ++k)
            {
                if (vr[k] != VR_RESET)
                    throw std::out_of_range("no Boolean variable with value reference "
                                            + std::to_string(vr[k]));
                i.reset = value[k];
            }
            return fmi2OK;
        });
    }

    fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                             fmi2String value[])
    {
        return guarded(c, "fmi2GetString", [&](Instance& i) {
            for (size_t k = 0; k < nvr; ++k)
            {
                if (vr[k] != VR_SOLVER)
                    throw std::out_of_range("no String variable with value reference "
                                            + std::to_string(vr[k]));
                value[k] = i.solver.c_str();
            }
            return fmi2OK;
        });
    }

    fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                             const fmi2String value[])
    {
        return guarded(c, "fmi2SetString", [&](Instance& i) {
            for (size_t k = 0; k < nvr; ++k)
            {
                if (vr[k] != VR_SOLVER)
                    throw std::out_of_range("no String variable with value reference "
                                            + std::to_string(vr[k]));
                if (i.sim)
                    throw std::logic_error("solver is fixed after initialization");
                const std::string s = value[k] ? value[k] : "";
                if (s != "euler" and s != "rk4" and s != "rkck")
                    throw std::invalid_argument("unknown solver '" + s + "'");
                i.solver = s;
            }
            return fmi2OK;
        });
    }

    fmi2Status fmi2DoStep(fmi2Component c, fmi2Real t, fmi2Real h, fmi2Boolean)
    {
        return guarded(c, "fmi2DoStep", [&](Instance& i) {
            if (not(i.sim))
                throw std::logic_error("fmi2DoStep before fmi2ExitInitializationMode");
            if (not(h > 0))
                throw std::invalid_argument("the communication step must be > 0");
            if (i.reset)
            {
                // The vessel jumps: the states before do not lead to this one.
                i.state = i.reset_state;
                i.reset = false;
                i.sim->sim.reset_history();
            }
            i.step(t, h);
            return fmi2OK;
        });
    }

    // fmi2DoStep returns once the step is done: there is never a step to cancel or to report on.
    fmi2Status fmi2CancelStep(fmi2Component) { return fmi2Error; }
    fmi2Status fmi2GetStatus(fmi2Component, const fmi2StatusKind, fmi2Status*)
    {
        return fmi2Discard;
    }
    fmi2Status fmi2GetRealStatus(fmi2Component, const fmi2StatusKind, fmi2Real*)
    {
        return fmi2Discard;
    }
    fmi2Status fmi2GetIntegerStatus(fmi2Component, const fmi2StatusKind, fmi2Integer*)
    {
        return fmi2Discard;
    }
    fmi2Status fmi2GetBooleanStatus(fmi2Component, const fmi2StatusKind, fmi2Boolean*)
    {
        return fmi2Discard;
    }
    fmi2Status fmi2GetStringStatus(fmi2Component, const fmi2StatusKind, fmi2String*)
    {
        return fmi2Discard;
    }

    // Declared false in modelDescription.xml (canInterpolateInputs, maxOutputDerivativeOrder,
    // canGetAndSetFMUstate, providesDirectionalDerivative): an importer does not call these.
    fmi2Status fmi2SetRealInputDerivatives(fmi2Component, const fmi2ValueReference[], size_t,
                                           const fmi2Integer[], const fmi2Real[])
    {
        return fmi2Error;
    }
    fmi2Status fmi2GetRealOutputDerivatives(fmi2Component, const fmi2ValueReference[], size_t,
                                            const fmi2Integer[], fmi2Real[])
    {
        return fmi2Error;
    }
    fmi2Status fmi2GetFMUstate(fmi2Component, fmi2FMUstate*) { return fmi2Error; }
    fmi2Status fmi2SetFMUstate(fmi2Component, fmi2FMUstate) { return fmi2Error; }
    fmi2Status fmi2FreeFMUstate(fmi2Component, fmi2FMUstate*) { return fmi2Error; }
    fmi2Status fmi2SerializedFMUstateSize(fmi2Component, fmi2FMUstate, size_t*)
    {
        return fmi2Error;
    }
    fmi2Status fmi2SerializeFMUstate(fmi2Component, fmi2FMUstate, fmi2Byte[], size_t)
    {
        return fmi2Error;
    }
    fmi2Status fmi2DeSerializeFMUstate(fmi2Component, const fmi2Byte[], size_t, fmi2FMUstate*)
    {
        return fmi2Error;
    }
    fmi2Status fmi2GetDirectionalDerivative(fmi2Component, const fmi2ValueReference[], size_t,
                                            const fmi2ValueReference[], size_t, const fmi2Real[],
                                            fmi2Real[])
    {
        return fmi2Error;
    }
}
