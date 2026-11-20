#include "XdynForCS.hpp"

#include <functional>
#include <ssc/solver/solve.hpp>
#include <ssc/solver/steppers.hpp>

#include "CoSimulationObserver.hpp"
#include "JSONSerializer.hpp"
#include "SimServerInputs.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/simulator_api/simulator_api.hpp"

XdynForCS::XdynForCS(const std::string& yaml_model, const std::string& solver, const double dt)
    : builder(yaml_model), dt(dt), sim(builder.sim), solver(solver)
{
}

XdynForCS::XdynForCS(
    const std::string& yaml_model,
    const VectorOfVectorOfPoints& mesh,
    const std::string& solver,
    const double dt)
    : builder(yaml_model, mesh), dt(dt), sim(builder.sim), solver(solver)
{
}

std::vector<YamlState> XdynForCS::handle(const YamlSimServerInputs& request)
{
    return handle(SimServerInputs(request, builder.Tmax));
}

std::vector<YamlTimeSeries> to_YamlTimeSeries(const std::map<std::string, double>& commands);
std::vector<YamlTimeSeries> to_YamlTimeSeries(const std::map<std::string, double>& commands)
{
    std::vector<YamlTimeSeries> ret;
    for (const auto& key_value : commands) {
        YamlTimeSeries ts;
        ts.name = key_value.first;
        ts.t = std::vector<double>(1, 0);
        ts.values[key_value.first] = std::vector<double>(1, key_value.second);
        ret.push_back(ts);
    }
    return ret;
}

std::vector<YamlState> XdynForCS::handle(const SimServerInputs& request)
{
    if (request.Dt <= 0) {
        THROW(
            __PRETTY_FUNCTION__,
            InvalidInputException,
            "Dt should be greater than 0 but got Dt = " << request.Dt);
    }
    const double tstart = request.t;
    const double Dt = request.Dt;
    sim.reset_history();
    sim.set_bodystates(request.full_state_history);
    sim.set_command_listener(request.commands);
    CoSimulationObserver observer(request.requested_output, sim.get_bodies().at(0)->get_name());
    ssc::solver::Scheduler scheduler(tstart, tstart + Dt, dt);
    if (solver == "euler") {
        ssc::solver::quicksolve<ssc::solver::EulerStepper>(sim, scheduler, observer);
    } else if (solver == "rk4") {
        ssc::solver::quicksolve<ssc::solver::RK4Stepper>(sim, scheduler, observer);
    } else if (solver == "rkck") {
        ssc::solver::quicksolve<ssc::solver::RKCK>(sim, scheduler, observer);
    } else {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "unknown solver");
    }
    return observer.get();
}

double XdynForCS::get_Tmax() const
{
    return builder.Tmax;
}
