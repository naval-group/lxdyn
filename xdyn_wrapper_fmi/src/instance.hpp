#ifndef XDYN_WRAPPER_FMI_INSTANCE_HPP
#define XDYN_WRAPPER_FMI_INSTANCE_HPP

#include "fmi2Functions.h"
#include "model.hpp"

#include "xdyn/observers_and_api/ConfBuilder.hpp"

#include <array>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace xdyn_fmu
{
    struct Instance
    {
        std::string name;
        fmi2Type type = fmi2CoSimulation;
        fmi2CallbackFunctions callbacks{};
        bool logging = false;
        std::string resources;
        std::string yaml;
        double dt = 0.1;
        std::string solver = "rk4";
        std::unique_ptr<ConfBuilder> sim;
        // Why the model cannot be imported in Model Exchange, empty when it can.
        std::string no_model_exchange;
        double time = 0; // fmi2SetTime, or the end of the last fmi2DoStep
        // Time of the last state the history holds: a master going back before it (a restart
        // without fmi2Reset) starts a new history, xdyn's only goes forward.
        double history_time = -std::numeric_limits<double>::infinity();
        std::vector<std::string> command_names;
        std::vector<double> commands;
        std::array<double, 13> state{};
        std::array<double, 3> euler{};
        bool reset_input = false; // reset_state as the master set it
        bool reset = false;       // a reset to apply at the start of the next step
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
            command_names.clear();
            commands.clear();
            for (const auto& command : model.commands)
            {
                command_names.push_back(command.xdyn_name);
                commands.push_back(command.value);
            }
            state = model.state;
            euler = model.euler;
            no_model_exchange = model.no_model_exchange;
            reset_state = model.state;
            reset_input = false;
            reset = false;
            sim.reset();
            time = 0;
            history_time = -std::numeric_limits<double>::infinity();
        }

        // Built at the end of initialization rather than at instantiation: xdyn.dt and xdyn.solver
        // are parameters, set in between.
        void build_sim()
        {
            sim.reset(with_assets_root(resources, [&] { return new ConfBuilder(yaml); }));
        }

        // Co-Simulation (co_simulation.cpp): integrates the Sim from t to t + h.
        void step(double t, double h);
        // Model Exchange (model_exchange.cpp), and der(...) in Co-Simulation.
        StateType derivatives();
        std::array<double, 3> angles() const;
    };

    inline Instance* as(fmi2Component c) { return static_cast<Instance*>(c); }

    // Runs f on the instance, turning an exception into fmi2Error and a log message.
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
} // namespace xdyn_fmu

#endif
