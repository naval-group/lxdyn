// xdyn as an FMI 2.0 FMU: what Co-Simulation (co_simulation.cpp) and Model Exchange
// (model_exchange.cpp) share, from instantiation to the variables.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE // dladdr
#endif

#include "instance.hpp"
#include "xdyn_fmu.h"

#include <algorithm>
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
    // lines, the defaults of the xdyn.dt and xdyn.solver parameters.
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
            if (fmuType != fmi2CoSimulation and fmuType != fmi2ModelExchange)
                throw std::runtime_error("unknown FMU type");
            i->type = fmuType;
            if (not(resourceLocation))
                throw std::runtime_error("no resource location");
            i->resources = uri_to_path(resourceLocation);
            i->yaml = read_file(i->resources + "/model.yml");
            read_parameters(i->resources + "/parameters.txt", i->dt, i->solver);
            i->load_model();
            if (fmuType == fmi2ModelExchange and not(i->no_model_exchange.empty()))
                throw std::runtime_error("no Model Exchange: " + i->no_model_exchange);
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
                    value[k] = i.angles()[r - VR_EULER];
                else if (r >= VR_DERIVATIVE and r < VR_DERIVATIVE + 13
                         and i.no_model_exchange.empty())
                    value[k] = i.derivatives()[r - VR_DERIVATIVE];
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
                    // OpenModelica's importFMU sets every parameter again after initialization.
                    if (i.sim and value[k] != i.dt)
                        throw std::logic_error("xdyn.dt is fixed after initialization");
                    if (not(value[k] > 0))
                        throw std::invalid_argument("xdyn.dt must be > 0");
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

    // xdyn.solver, the one Enumeration: item k + 1 is solvers()[k].
    fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                              fmi2Integer value[])
    {
        return guarded(c, "fmi2GetInteger", [&](Instance& i) {
            for (size_t k = 0; k < nvr; ++k)
            {
                if (vr[k] != VR_SOLVER)
                    throw std::out_of_range("no Integer variable with value reference "
                                            + std::to_string(vr[k]));
                value[k] = static_cast<fmi2Integer>(
                    std::find(solvers().begin(), solvers().end(), i.solver) - solvers().begin()
                    + 1);
            }
            return fmi2OK;
        });
    }

    fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
                              const fmi2Integer value[])
    {
        return guarded(c, "fmi2SetInteger", [&](Instance& i) {
            for (size_t k = 0; k < nvr; ++k)
            {
                if (vr[k] != VR_SOLVER)
                    throw std::out_of_range("no Integer variable with value reference "
                                            + std::to_string(vr[k]));
                if (value[k] < 1 or value[k] > static_cast<fmi2Integer>(solvers().size()))
                    throw std::invalid_argument("no solver " + std::to_string(value[k]));
                const std::string& solver = solvers()[static_cast<size_t>(value[k] - 1)];
                if (i.sim and solver != i.solver)
                    throw std::logic_error("xdyn.solver is fixed after initialization");
                i.solver = solver;
            }
            return fmi2OK;
        });
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
                value[k] = i.reset_input;
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
                // A master sets every input before each step: only the edge asks for a reset.
                if (value[k] and not(i.reset_input))
                    i.reset = true;
                i.reset_input = value[k];
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
                if (vr[k] != VR_ENVIRONMENT)
                    throw std::out_of_range("no String variable with value reference "
                                            + std::to_string(vr[k]));
                value[k] = i.environment.c_str();
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
                if (vr[k] != VR_ENVIRONMENT)
                    throw std::out_of_range("no String variable with value reference "
                                            + std::to_string(vr[k]));
                const std::string environment = value[k] ? value[k] : "";
                if (i.sim and environment != i.environment)
                    throw std::logic_error("xdyn.environment is fixed after initialization");
                if (not(environment.empty()))
                    with_environment(i.yaml, environment); // fails here rather than at initialization
                i.environment = environment;
            }
            return fmi2OK;
        });
    }

    // Declared false in modelDescription.xml (canGetAndSetFMUstate,
    // providesDirectionalDerivative): an importer does not call these.
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
