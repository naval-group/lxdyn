// xdyn as an FMI 2.0 Co-Simulation FMU.
//
// Each instance holds an XdynForCS, the class behind xdyn-for-cs, and calls it the way the
// websocket server does: fmi2DoStep(t, h) is one request with the instance's state, its
// commands and Dt = h, so a step gives what xdyn-for-cs gives. The state stays in the FMU
// between steps. reset_state imposes another one at the start of the next step: a master may
// teleport a vessel or switch it to another model, and FMI 2.0 Co-Simulation has no other way
// to set a state (fmi2SetFMUstate is opaque).

#ifndef _GNU_SOURCE
#define _GNU_SOURCE // dladdr
#endif

#include "model.hpp"
#include "xdyn_fmu.h"

#include "xdyn/external_data_structures/YamlSimServerInputs.hpp"
#include "xdyn/external_data_structures/YamlState.hpp"
#include "xdyn/observers_and_api/XdynForCS.hpp"

#include <fstream>
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

    YamlState to_yaml_state(const double t, const std::array<double, 13>& s)
    {
        return YamlState(t, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10],
                         s[11], s[12]);
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
        std::unique_ptr<XdynForCS> sim;
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
        }

        // Built at the end of initialization rather than at instantiation: xdyn_dt and solver
        // are parameters, set in between.
        void build_sim()
        {
            sim.reset(with_assets_root(resources, [&] { return new XdynForCS(yaml, solver, dt); }));
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
            if (i.reset)
            {
                i.state = i.reset_state;
                i.reset = false;
            }
            YamlSimServerInputs request;
            request.Dt = h;
            request.states.push_back(to_yaml_state(t, i.state));
            for (size_t k = 0; k < i.commands.size(); ++k)
                request.commands[i.command_names[k]] = i.commands[k];
            const std::vector<YamlState> out = i.sim->handle(request);
            if (out.empty())
                throw std::runtime_error("xdyn returned no state");
            const YamlState& s = out.back();
            i.state = { s.x, s.y, s.z, s.u, s.v, s.w, s.p, s.q, s.r, s.qr, s.qi, s.qj, s.qk };
            i.euler = { s.phi, s.theta, s.psi };
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
