// Model Exchange: the importer integrates the 13 states of the body, and fmi2GetDerivatives is
// Sim::dx_dt. modelDescription.xml only offers it for one body, no history and no controllers
// (see read_model), which is what makes a derivative a function of the time, the states and the
// commands alone.

#include "instance.hpp"

#include <algorithm>
#include <map>

using namespace xdyn_fmu;

StateType xdyn_fmu::Instance::derivatives()
{
    if (not(sim))
        throw std::logic_error("no derivatives before fmi2ExitInitializationMode");
    Sim& s = sim->sim;
    // dx_dt records the state it is given, and an importer goes back in time when it rejects a
    // step: History::record would throw. No force model reads that history here.
    s.reset_history();
    std::map<std::string, double> values;
    for (size_t k = 0; k < commands.size(); ++k)
        values[command_names[k]] = commands[k];
    s.set_command_listener(values);
    const StateType x(state.begin(), state.end());
    StateType dx_dt(x.size());
    s.dx_dt(x, dx_dt, time);
    return dx_dt;
}

std::array<double, 3> xdyn_fmu::Instance::angles() const
{
    // In Co-Simulation they come with the state at the end of each step.
    if (type == fmi2CoSimulation or not(sim))
        return euler;
    const StateType x(state.begin(), state.end());
    const auto a = sim->sim.get_bodies().at(0)->get_angles(x, sim->sim.get_env().rot);
    return { a.phi, a.theta, a.psi };
}

extern "C"
{
    fmi2Status fmi2EnterEventMode(fmi2Component c) { return c ? fmi2OK : fmi2Error; }

    // No events: the model has no event indicator and no time event.
    fmi2Status fmi2NewDiscreteStates(fmi2Component c, fmi2EventInfo* info)
    {
        if (not(c) or not(info))
            return fmi2Error;
        *info = fmi2EventInfo{};
        return fmi2OK;
    }

    fmi2Status fmi2EnterContinuousTimeMode(fmi2Component c) { return c ? fmi2OK : fmi2Error; }

    fmi2Status fmi2CompletedIntegratorStep(fmi2Component c, fmi2Boolean,
                                           fmi2Boolean* enterEventMode,
                                           fmi2Boolean* terminateSimulation)
    {
        if (not(c))
            return fmi2Error;
        if (enterEventMode)
            *enterEventMode = fmi2False;
        if (terminateSimulation)
            *terminateSimulation = fmi2False;
        return fmi2OK;
    }

    fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time)
    {
        if (not(c))
            return fmi2Error;
        as(c)->time = time;
        return fmi2OK;
    }

    fmi2Status fmi2SetContinuousStates(fmi2Component c, const fmi2Real x[], size_t nx)
    {
        return guarded(c, "fmi2SetContinuousStates", [&](Instance& i) {
            if (nx != i.state.size())
                throw std::invalid_argument("expected 13 states, got " + std::to_string(nx));
            std::copy(x, x + nx, i.state.begin());
            return fmi2OK;
        });
    }

    fmi2Status fmi2GetContinuousStates(fmi2Component c, fmi2Real x[], size_t nx)
    {
        return guarded(c, "fmi2GetContinuousStates", [&](Instance& i) {
            if (nx != i.state.size())
                throw std::invalid_argument("expected 13 states, got " + std::to_string(nx));
            std::copy(i.state.begin(), i.state.end(), x);
            return fmi2OK;
        });
    }

    fmi2Status fmi2GetDerivatives(fmi2Component c, fmi2Real derivatives[], size_t nx)
    {
        return guarded(c, "fmi2GetDerivatives", [&](Instance& i) {
            if (nx != i.state.size())
                throw std::invalid_argument("expected 13 derivatives, got " + std::to_string(nx));
            const StateType dx_dt = i.derivatives();
            std::copy(dx_dt.begin(), dx_dt.end(), derivatives);
            return fmi2OK;
        });
    }

    fmi2Status fmi2GetEventIndicators(fmi2Component c, fmi2Real[], size_t ni)
    {
        return c and ni == 0 ? fmi2OK : fmi2Error;
    }

    fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component c, fmi2Real x_nominal[], size_t nx)
    {
        if (not(c) or nx != 13)
            return fmi2Error;
        std::fill(x_nominal, x_nominal + nx, 1.0);
        return fmi2OK;
    }
}
