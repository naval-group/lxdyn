/*
 * ODEMocks.hpp
 *
 * \date 19 mars 2014
 *      Author: cec
 */

#ifndef ODEMOCKS_HPP_
#define ODEMOCKS_HPP_

#include "gmock/gmock.h"

#include <boost/numeric/odeint/stepper/controlled_step_result.hpp>
#include <functional>
#include <memory>
#include <vector>

#include "ssc/solver/ContinuousSystem.hpp"
#include "ssc/solver/EventHandler.hpp"
#include "ssc/solver/Scheduler.hpp"
#include "ssc/solver/DiscreteSystem.hpp"

class SystemWithMock;

class ODEMocks
{
  public:
    ODEMocks() {}
    MOCK_METHOD0(advance_to_next_time_event, void());
    MOCK_METHOD0(flush, void());
    MOCK_METHOD0(check_variables_to_serialize_are_available, void());
    MOCK_METHOD3(model_function, void(const std::vector<double> &, std::vector<double> &, double));
    MOCK_METHOD3(observe_before_solver_step, void(const ssc::solver::ContinuousSystem &, const double, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >&));
    MOCK_METHOD3(observe_after_solver_step, void(const ssc::solver::ContinuousSystem &, const double, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >&));
    MOCK_METHOD3(collect_available_serializations, void(const ssc::solver::ContinuousSystem &, const double, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >&));
    MOCK_METHOD4(do_step,
                 void(ssc::solver::ContinuousSystem &sys, std::vector<double> &, double, double));
    MOCK_METHOD4(try_step,
                 boost::numeric::odeint::controlled_step_result(ssc::solver::ContinuousSystem &sys,
                                                                std::vector<double> &, double,
                                                                double));
    MOCK_CONST_METHOD0(has_more_time_events, bool());
    MOCK_CONST_METHOD0(get_time, double());
    MOCK_CONST_METHOD0(get_step, double());
    MOCK_CONST_METHOD0(detected_state_events, bool());
    MOCK_CONST_METHOD1(add_time_event, void(const double));
    MOCK_CONST_METHOD0(locate_event, void());
    MOCK_CONST_METHOD0(run_event_actions, void());
    MOCK_CONST_METHOD2(force_states, void(std::vector<double> &, const double));
    MOCK_METHOD0(initialize_system_outputs_before_first_observation, void());
    MOCK_METHOD2(set_discrete_state, void(const std::string &, const double &));
    MOCK_CONST_METHOD1(get_input_value, double(const std::string &));
    MOCK_CONST_METHOD1(get_state_value, double(const std::string &));
};

class SystemWithMock : public ssc::solver::ContinuousSystem
{
  public:
    SystemWithMock(ODEMocks &mock_)
        : mock(mock_)
    {
    }
    void dx_dt(const std::vector<double> &x, std::vector<double> &dx_dt, double t)
    {
        mock.model_function(x, dx_dt, t);
    }

    void force_states(std::vector<double> &x, const double t) const { mock.force_states(x, t); }

    void initialize_system_outputs_before_first_observation()
    {
        mock.initialize_system_outputs_before_first_observation();
    }

    void set_discrete_state(const std::string &discrete_state_name, const double value)
    {
        mock.set_discrete_state(discrete_state_name, value);
    }

    double get_input_value(const std::string &name) const { mock.get_input_value(name); }

    double get_state_value(const std::string &name) const { mock.get_state_value(name); }

  protected:
    ODEMocks &mock;

  private:
    SystemWithMock();
};

class ObserverWithMock
{
  public:
    ObserverWithMock(ODEMocks &mock_)
        : mock(mock_)
    {
    }
    void collect_available_serializations(const ssc::solver::ContinuousSystem &s, const double d, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& ds) { mock.collect_available_serializations(s, d, ds); }
    void check_variables_to_serialize_are_available() const {mock.check_variables_to_serialize_are_available();}
    void flush() const {mock.flush();}
    void observe_before_solver_step(const ssc::solver::ContinuousSystem &s, const double d, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& ds) { mock.observe_before_solver_step(s, d, ds); }
    void observe_after_solver_step(const ssc::solver::ContinuousSystem &s, const double d, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& ds) { mock.observe_after_solver_step(s, d, ds); }

  private:
    ObserverWithMock();
    ODEMocks &mock;
};

class SchedulerWithMock : public ssc::solver::Scheduler
{
  public:
    SchedulerWithMock(ODEMocks &mock_)
        : ssc::solver::Scheduler(0, 0, 0)
        , mock(mock_)
        , first_run(new bool(true))
    {
    }
    bool has_more_time_events() const
    {
        mock.has_more_time_events();
        const bool b = *first_run;
        *first_run = false;
        return b;
    }

    void add_time_event(const double t) { mock.add_time_event(t); }

    ~SchedulerWithMock() noexcept { delete first_run; }

    SchedulerWithMock &operator=(const SchedulerWithMock &rhs)
    {
        if (this != &rhs)
        {
            *first_run = *(rhs.first_run);
        }
        return *this;
    }
    std::vector<SchedulerWithMock::Callback> get_discrete_state_updaters_to_run()
    {
        return std::vector<Callback>();
    }

    SchedulerWithMock(const SchedulerWithMock &rhs)
        : ssc::solver::Scheduler(0, 0, 0)
        , mock(rhs.mock)
        , first_run(new bool(*(rhs.first_run)))
    {
    }

    double get_time() const { return mock.get_time(); }

    double get_step() const { return mock.get_step(); }

    void advance_to_next_time_event() { mock.advance_to_next_time_event(); }

  private:
    SchedulerWithMock();
    ODEMocks &mock;
    bool *first_run;
};

class EventHandlerWithMock : public ssc::solver::EventHandler
{
  public:
    EventHandlerWithMock(ODEMocks &mock_)
        : mock(mock_)
    {
    }

    bool detected_state_events() const { return mock.detected_state_events(); }

    void locate_event() { mock.locate_event(); }

    void run_event_actions() { mock.run_event_actions(); }

  private:
    EventHandlerWithMock();
    ODEMocks &mock;
};

class ExplicitStepperWithMock
{
  public:
    ExplicitStepperWithMock(ODEMocks &mock_)
        : mock(mock_)
    {
    }
    void do_step(SystemWithMock &sys, std::vector<double> &x, double t, double dt)
    {
        mock.do_step(sys, x, t, dt);
    }

  private:
    ExplicitStepperWithMock();
    ODEMocks &mock;
};

class ControlledStepperWithMock
{
  public:
    ControlledStepperWithMock(ODEMocks &mock_)
        : mock(mock_)
    {
    }

    boost::numeric::odeint::controlled_step_result
    try_step(SystemWithMock &sys, std::vector<double> &x, double t, double dt)
    {
        return mock.try_step(sys, x, t, dt);
    }

  private:
    ControlledStepperWithMock();
    ODEMocks &mock;
};

#endif /* ODEMOCKS_HPP_ */
