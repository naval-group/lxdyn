/*
 * ODEintTests.cpp
 *
 * \date February 23 2021
 *      Author: CEC
 */

#include "DiscreteSystemsTests.hpp"

#include "ssc/solver/DiscreteSystem.hpp"
#include "ssc/solver/Observers.hpp"
#include "ssc/solver/Scheduler.hpp"
#include "ssc/solver/solve.hpp"
#include "test_systems.hpp"
#include <functional> // Otherwise CLang issues an error (bind.hpp:36:14: error: ‘std::bind’ has not been declared)

#include <boost/numeric/odeint/stepper/euler.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta4.hpp>

#include <sstream>

DiscreteSystemsTests::DiscreteSystemsTests()
    : a(ssc::random_data_generator::DataGenerator(36))
{
}

DiscreteSystemsTests::~DiscreteSystemsTests() {}

void DiscreteSystemsTests::SetUp() {}

void DiscreteSystemsTests::TearDown() {}

typedef boost::numeric::odeint::euler<StateType> EulerStepper;
typedef boost::numeric::odeint::runge_kutta4<StateType> RungeKuttaStepper;

struct ContinuousSystem : public ssc::solver::ContinuousSystem
{
    ContinuousSystem(const std::vector<double> &x0) { state = x0; }

    void dx_dt(const std::vector<double> &, std::vector<double> &dxdt, double)
    {
        dxdt[0] = 1;
    }

    void initialize_system_outputs_before_first_observation() {}
    void set_discrete_state(const std::string &, const double) {}
    double get_input_value(const std::string &) const { return -1; }
    double get_state_value(const std::string &) const { return -1; }
};

class DiscreteSystem : public ssc::solver::DiscreteSystem
{
  public:
    DiscreteSystem(const double tstart_, const double dt_)
        : ssc::solver::DiscreteSystem(tstart_, dt_)
        , ticks({ 13 })
        , calltimes()
    {
        
    }
    std::vector<size_t> ticks;
    std::vector<double> calltimes;

  private:
    void update_discrete_states(const double t, ssc::solver::ContinuousSystem&) override
    {
        ticks.push_back(ticks.back() + 1);
        calltimes.push_back(t);
    }
};

class DiscreteSystemThatStopsAfterAWhile : public ssc::solver::DiscreteSystem
{
  public:
    DiscreteSystemThatStopsAfterAWhile(const double tstart_, const double dt_, const double tmax_)
        : ssc::solver::DiscreteSystem(tstart_, dt_)
        , calltimes()
        , ddt(dt_)
        , ttstart(tstart_)
        , tmax(tmax_)
    {
    }
    std::vector<double> calltimes;
    double ddt;
    double ttstart;
    double tmax;

    private :
        void update_discrete_states(const double t, ssc::solver::ContinuousSystem&) override
        {
            calltimes.push_back(t);
        }
        double get_date_of_next_update(const double current_time) const override
        {
            if (current_time >= tmax) return current_time;
            const int current_index = (int)std::round((current_time - ttstart) / ddt);
            return ttstart + (current_index + 1) * ddt;
        }
};

TEST_F(DiscreteSystemsTests, can_initialize_discrete_states)
{
    ssc::solver::VectorObserver observer;
    const double t0 = 6;
    const double dt = 2;
    ssc::solver::Scheduler scheduler(t0, t0 + 4 * dt, dt);
    ContinuousSystem continuous_system(std::vector<double>(1, 0));
    DiscreteSystem discrete_system(t0, dt / 2);
    discrete_system.initialize(scheduler, continuous_system);
    // Check discrete_system has been initialized
    ASSERT_EQ(std::vector<double>({ t0 }), discrete_system.calltimes);
    ASSERT_EQ(14, discrete_system.ticks.back());
    // Scheduler is still at t=t0, no discrete state updaters left to run.
    ASSERT_EQ(t0, scheduler.get_time());
    ASSERT_EQ(0, scheduler.get_discrete_state_updaters_to_run().size());
    // Discrete system callback has been added to the scheduler.
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(t0 + dt / 2, scheduler.get_time());
    ASSERT_EQ(1, scheduler.get_discrete_state_updaters_to_run().size());
}

TEST_F(DiscreteSystemsTests, one_second_steps)
{
    ssc::solver::VectorObserver observer;
    ContinuousSystem system(std::vector<double>(1, 0));

    const double t0 = 6;
    const double tend = 16.1;
    const double dt = 0.01;
    ssc::solver::Scheduler scheduler(t0, tend, dt);

    const double discrete_system_dt = 1;
    DiscreteSystem discrete_system(t0, discrete_system_dt);
    discrete_system.schedule_update(7, scheduler);

    ssc::solver::quicksolve<EulerStepper>(system, scheduler, observer);

    ASSERT_EQ(std::vector<double>({ 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }),
              discrete_system.calltimes);
    ASSERT_EQ(13 + discrete_system.calltimes.size(), discrete_system.ticks.back());
}

TEST_F(DiscreteSystemsTests, quarter_second_steps)
{
    ssc::solver::VectorObserver observer;
    ContinuousSystem continuous_system(std::vector<double>(1, 0));

    const double t0 = 6;
    const double tend = 10;
    const double dt = 0.2;
    ssc::solver::Scheduler scheduler(t0, tend, dt);

    DiscreteSystem discrete_system(t0, 0.3);
    discrete_system.schedule_update(7, scheduler);

    ssc::solver::quicksolve<EulerStepper>(continuous_system, scheduler, observer);
    const auto observations = observer.get();

    // Check discrete system calltimes
    const std::vector<double> expected_discrete_system_calltimes
        = { 7, 7.2, 7.5, 7.8, 8.1, 8.4, 8.7, 9, 9.3, 9.6, 9.9 };
    ASSERT_EQ(expected_discrete_system_calltimes.size(), discrete_system.calltimes.size());
    for (size_t i = 0; i < expected_discrete_system_calltimes.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_discrete_system_calltimes.at(i), discrete_system.calltimes.at(i));
    }
    // Check discrete system values
    ASSERT_EQ(13 + discrete_system.calltimes.size(), discrete_system.ticks.back());

    // Check continuous system calltimes
    const std::vector<double> expected_observation_instants
        = { 6,   6.2, 6.4, 6.6, 6.8, 7, 7.2, 7.4, 7.5, 7.6, 7.8, 8,   8.1,
            8.2, 8.4, 8.6, 8.7, 8.8, 9, 9.2, 9.3, 9.4, 9.6, 9.8, 9.9, 10 };
    ASSERT_EQ(expected_observation_instants.size(), observations.size());
    for (size_t i = 0; i < observations.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_observation_instants.at(i), observations.at(i).first);
    }
    // Check continuous system values
    for (const auto &observation : observations)
    {
        ASSERT_DOUBLE_EQ(observation.first - t0, observation.second);
    }
}

TEST_F(DiscreteSystemsTests, quarter_second_steps_with_shifted_start)
{
    ssc::solver::VectorObserver observer;
    ContinuousSystem continuous_system(std::vector<double>(1, 0));

    const double t0 = 6;
    const double tend = 10;
    const double dt = 0.2;
    ssc::solver::Scheduler scheduler(t0, tend, dt);

    DiscreteSystem discrete_system(7.325, 0.3);
    discrete_system.schedule_update(7.325, scheduler);

    ssc::solver::quicksolve<EulerStepper>(continuous_system, scheduler, observer);
    const auto observations = observer.get();

    // Check discrete system calltimes
    const std::vector<double> expected_discrete_system_calltimes
        = { 7.325, 7.625, 7.925, 8.225, 8.525, 8.825, 9.125, 9.425, 9.725 };
    ASSERT_EQ(expected_discrete_system_calltimes.size(), discrete_system.calltimes.size());
    for (size_t i = 0; i < expected_discrete_system_calltimes.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_discrete_system_calltimes.at(i), discrete_system.calltimes.at(i));
    }
    // Check discrete system values
    ASSERT_EQ(13 + discrete_system.calltimes.size(), discrete_system.ticks.back());

    // Check continuous system calltimes
    const std::vector<double> expected_observation_instants
        = { 6,     6.2, 6.4,   6.6, 6.8, 7,     7.2, 7.325, 7.4, 7.6, 7.625, 7.8, 7.925, 8,   8.2,
            8.225, 8.4, 8.525, 8.6, 8.8, 8.825, 9,   9.125, 9.2, 9.4, 9.425, 9.6, 9.725, 9.8, 10 };
    ASSERT_EQ(expected_observation_instants.size(), observations.size());
    for (size_t i = 0; i < observations.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_observation_instants.at(i), observations.at(i).first);
    }
    // Check continuous system values
    for (const auto &observation : observations)
    {
        ASSERT_DOUBLE_EQ(observation.first - t0, observation.second);
    }
}

TEST_F(DiscreteSystemsTests, quarter_second_steps_with_shifted_start_and_discrete_system_initialized_at_t0)
{
    ssc::solver::VectorObserver observer;
    ContinuousSystem continuous_system(std::vector<double>(1, 0));

    const double t0 = 6;
    const double tend = 10;
    const double dt = 0.2;
    ssc::solver::Scheduler scheduler(t0, tend, dt);

    DiscreteSystem discrete_system(t0, 0.3);
    discrete_system.schedule_update(7.325, scheduler);

    ssc::solver::quicksolve<EulerStepper>(continuous_system, scheduler, observer);
    const auto observations = observer.get();

    // Check discrete system calltimes
    const std::vector<double> expected_discrete_system_calltimes
        = { 7.325, 7.5, 7.8, 8.1, 8.4, 8.7, 9, 9.3, 9.6, 9.9 };
    ASSERT_EQ(expected_discrete_system_calltimes.size(), discrete_system.calltimes.size());
    for (size_t i = 0; i < expected_discrete_system_calltimes.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_discrete_system_calltimes.at(i), discrete_system.calltimes.at(i));
    }
    // Check discrete system values
    ASSERT_EQ(13 + discrete_system.calltimes.size(), discrete_system.ticks.back());

    // Check continuous system calltimes
    const std::vector<double> expected_observation_instants
        = { 6,   6.2, 6.4, 6.6, 6.8, 7, 7.2, 7.325, 7.4, 7.5, 7.6, 7.8, 8, 8.1,
            8.2, 8.4, 8.6, 8.7, 8.8, 9, 9.2, 9.3,   9.4, 9.6, 9.8, 9.9, 10 };
    ASSERT_EQ(expected_observation_instants.size(), observations.size());
    for (size_t i = 0; i < observations.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_observation_instants.at(i), observations.at(i).first);
    }
    // Check continuous system values
    for (const auto &observation : observations)
    {
        ASSERT_DOUBLE_EQ(observation.first - t0, observation.second);
    }
}

TEST_F(DiscreteSystemsTests, quarter_second_steps_with_rk4)
{
    ssc::solver::VectorObserver observer;
    ContinuousSystem continuous_system(std::vector<double>(1, 0));

    const double t0 = 6;
    const double tend = 10;
    const double dt = 0.2;
    ssc::solver::Scheduler scheduler(t0, tend, dt);

    DiscreteSystem discrete_system(7.325, 0.3);
    discrete_system.schedule_update(7.325, scheduler);

    ssc::solver::quicksolve<RungeKuttaStepper>(continuous_system, scheduler, observer);
    const auto observations = observer.get();

    // Check discrete system calltimes
    const std::vector<double> expected_discrete_system_calltimes
        = { 7.325, 7.625, 7.925, 8.225, 8.525, 8.825, 9.125, 9.425, 9.725 };
    ASSERT_EQ(expected_discrete_system_calltimes.size(), discrete_system.calltimes.size());
    for (size_t i = 0; i < expected_discrete_system_calltimes.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_discrete_system_calltimes.at(i), discrete_system.calltimes.at(i));
    }
    // Check discrete system values
    ASSERT_EQ(13 + discrete_system.calltimes.size(), discrete_system.ticks.back());

    // Check continuous system calltimes
    const std::vector<double> expected_observation_instants
        = { 6,     6.2, 6.4,   6.6, 6.8, 7,     7.2, 7.325, 7.4, 7.6, 7.625, 7.8, 7.925, 8,   8.2,
            8.225, 8.4, 8.525, 8.6, 8.8, 8.825, 9,   9.125, 9.2, 9.4, 9.425, 9.6, 9.725, 9.8, 10 };
    ASSERT_EQ(expected_observation_instants.size(), observations.size());
    for (size_t i = 0; i < observations.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_observation_instants.at(i), observations.at(i).first);
    }
    // Check continuous system values
    for (const auto &observation : observations)
    {
        ASSERT_DOUBLE_EQ(observation.first - t0, observation.second);
    }
}

TEST_F(DiscreteSystemsTests, no_time_drift)
{
    ssc::solver::VectorObserver observer;
    ContinuousSystem system(std::vector<double>(1, 0));

    const double t0 = 0;
    const double tend = 0.11;
    const double dt = 0.1;
    ssc::solver::Scheduler scheduler(t0, tend, dt);

    DiscreteSystem discrete_system(t0, 1e-4);
    discrete_system.schedule_update(t0, scheduler);

    ssc::solver::quicksolve<EulerStepper>(system, scheduler, observer);

    // Check discrete system calltimes
    const std::vector<double> expected_discrete_system_calltimes
        = { 0,
            0.1000, 0.1001, 0.1002, 0.1003, 0.1004, 0.1005, 0.1006, 0.1007, 0.1008, 0.1009,
            0.1010, 0.1011, 0.1012, 0.1013, 0.1014, 0.1015, 0.1016, 0.1017, 0.1018, 0.1019,
            0.1020, 0.1021, 0.1022, 0.1023, 0.1024, 0.1025, 0.1026, 0.1027, 0.1028, 0.1029,
            0.1030, 0.1031, 0.1032, 0.1033, 0.1034, 0.1035, 0.1036, 0.1037, 0.1038, 0.1039,
            0.1040, 0.1041, 0.1042, 0.1043, 0.1044, 0.1045, 0.1046, 0.1047, 0.1048, 0.1049,
            0.1050, 0.1051, 0.1052, 0.1053, 0.1054, 0.1055, 0.1056, 0.1057, 0.1058, 0.1059,
            0.1060, 0.1061, 0.1062, 0.1063, 0.1064, 0.1065, 0.1066, 0.1067, 0.1068, 0.1069,
            0.1070, 0.1071, 0.1072, 0.1073, 0.1074, 0.1075, 0.1076, 0.1077, 0.1078, 0.1079,
            0.1080, 0.1081, 0.1082, 0.1083, 0.1084, 0.1085, 0.1086, 0.1087, 0.1088, 0.1089,
            0.1090, 0.1091, 0.1092, 0.1093, 0.1094, 0.1095, 0.1096, 0.1097, 0.1098 }; // 0.1099 is not scheduled because it's too close to tend==0.11
    ASSERT_EQ(expected_discrete_system_calltimes.size(), discrete_system.calltimes.size());
    for (size_t i = 0; i < expected_discrete_system_calltimes.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(expected_discrete_system_calltimes.at(i), discrete_system.calltimes.at(i));
    }
}

TEST_F(DiscreteSystemsTests, discrete_system_that_stops_updating_after_a_while)
{
    ssc::solver::VectorObserver observer;
    ContinuousSystem system(std::vector<double>(1, 0));

    const double t0 = 2;
    const double tend = 16.1;
    const double dt = 0.1;
    ssc::solver::Scheduler scheduler(t0, tend, dt);

    const double discrete_system_dt = 0.05;
    DiscreteSystemThatStopsAfterAWhile discrete_system(t0, discrete_system_dt, 2.4);
    discrete_system.schedule_update(t0, scheduler);

    ssc::solver::quicksolve<EulerStepper>(system, scheduler, observer);

    // Discrete system should have been called every 'discrete_system_dt', from t0=2s to 2.4s
    ASSERT_EQ(std::vector<double>({ 2,2.05,2.1,2.15,2.20,2.25,2.3,2.35,2.4 }),
              discrete_system.calltimes);
}