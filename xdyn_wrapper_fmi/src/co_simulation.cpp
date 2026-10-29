// Co-Simulation: each instance owns a Sim and integrates it from t to t + h at each
// fmi2DoStep, with xdyn's own solver and step, as the xdyn executable does over a whole
// simulation: the history the force models read (radiation damping) goes on from one step to
// the next. reset_state imposes another state at the start of the next step when it goes from
// false to true: a master may teleport a vessel or switch it to another model, and FMI 2.0
// Co-Simulation has no other way to set a state (fmi2SetFMUstate is opaque).

#include "instance.hpp"

#include "xdyn/external_data_structures/YamlState.hpp"
#include "xdyn/observers_and_api/CoSimulationObserver.hpp"
#include "xdyn/observers_and_api/simulator_api.hpp"

#include <ssc/solver/steppers.hpp>

#include <algorithm>
#include <cmath>
#include <map>

using namespace xdyn_fmu;

void xdyn_fmu::Instance::step(const double t, const double h)
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

extern "C"
{
    fmi2Status fmi2DoStep(fmi2Component c, fmi2Real t, fmi2Real h, fmi2Boolean)
    {
        return guarded(c, "fmi2DoStep", [&](Instance& i) {
            if (i.type != fmi2CoSimulation)
                throw std::logic_error("fmi2DoStep on a Model Exchange instance");
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
            i.time = t + h;
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

    // Declared false in modelDescription.xml (canInterpolateInputs, maxOutputDerivativeOrder):
    // an importer does not call these.
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
}
