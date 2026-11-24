/*
 * ControllerTests.cpp
 */

#include <boost/algorithm/string.hpp>
#include <ssc/kinematics.hpp>
#include <ssc/solver/Scheduler.hpp>
#include <tuple>

#include "xdyn/controllers/CSVController.hpp"
#include "xdyn/controllers/PIDController.hpp"
#include "xdyn/controllers/unit_tests/CSVControllerTest.hpp"
#include "xdyn/controllers/unit_tests/PIDControllerTest.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/simulator_api/simulator_api.hpp"

TEST_F(PIDControllerTest, update_command_in_ds_example)
{
    const double tstart = 0.1;
    const double dt = 0.5;
    ssc::solver::Scheduler scheduler(tstart, 10, dt);

    const double Kp = 2.5;
    const double Ki = 0.1;
    const double Kd = 0.314;
    PIDController controller(
        tstart,
        dt,
        a.random<std::string>(),
        pid_specific_yaml(Kp, Ki, Kd, "rpm_co", "propeller(rpm)") +
            "state weights:\n    x: 1\n    y: -1\n");

    const double rpm_co = 5;
    const double x = 2 * rpm_co;
    const double y = rpm_co + 0.05;

    Sim sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);

    //! [callback example]
    // First time step
    sys.set_command_listener(std::map<std::string, double>({{"t", tstart}, {"rpm_co", rpm_co}}));
    StateType states = {x, y, 300.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 1, 0, 0, 0};
    sys.get_bodies().front()->update_body_states(states, tstart);

    controller.callback(scheduler, sys);
    const double error = rpm_co - (x - y);
    ASSERT_NEAR(Kp * error, sys.get_input_value("propeller(rpm)"), 1e-6);

    //! [callback example]
}

TEST_F(PIDControllerTest, can_compute_PID_commands)
{
    const double tstart = 0.1;
    const double dt = 0.5;
    ssc::solver::Scheduler scheduler(tstart, 2, dt);

    const double Kp = 2.5;
    const double Ki = 0.1;
    const double Kd = 0.314;
    PIDController controller(
        tstart,
        dt,
        a.random<std::string>(),
        pid_specific_yaml(Kp, Ki, Kd, "rpm_co", "propeller(rpm)") +
            "state weights:\n    x: 1\n    y: -1\n");

    Sim sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);

    // First time step
    ASSERT_EQ(tstart, scheduler.get_time());
    const double error = 0.05;
    double rpm_co = 5;
    double x = 2 * rpm_co;
    double y = rpm_co + error;
    sys.set_command_listener(std::map<std::string, double>({{"t", tstart}, {"rpm_co", rpm_co}}));

    StateType states = {x, y, 300.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 1, 0, 0, 0};
    sys.get_bodies().front()->update_body_states(states, tstart);

    controller.callback(scheduler, sys);
    const double first_expected_command = Kp * error;
    ASSERT_NEAR(first_expected_command, sys.get_input_value("propeller(rpm)"), 1e-6);

    // Second time step
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(tstart + dt, scheduler.get_time());
    const double error2 = -0.03;
    rpm_co = 1.5;
    x = 2 * rpm_co;
    y = rpm_co + error2;
    sys.set_command_listener(
        std::map<std::string, double>({{"t", tstart + dt}, {"rpm_co", rpm_co}}));
    states = {x, y, 300.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 1, 0, 0, 0};
    sys.get_bodies().front()->update_body_states(states, tstart + dt);

    controller.callback(scheduler, sys);
    const double second_expected_command =
        Kp * error2 + Ki * error2 * dt + Kd * (error2 - error) / dt;
    ASSERT_NEAR(second_expected_command, sys.get_input_value("propeller(rpm)"), 1e-6);

    // Third time step
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(tstart + 2 * dt, scheduler.get_time());
    const double error3 = 0.01;
    rpm_co = 5.5;
    x = 2 * rpm_co;
    y = rpm_co + error3;
    sys.set_command_listener(
        std::map<std::string, double>({{"t", tstart + 2 * dt}, {"rpm_co", rpm_co}}));
    states = {x, y, 300.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 1, 0, 0, 0};
    sys.get_bodies().front()->update_body_states(states, tstart + 2 * dt);

    controller.callback(scheduler, sys);
    const double third_expected_command =
        Kp * error3 + Ki * (error2 * dt + error3 * dt) + Kd * (error3 - error2) / dt;
    ASSERT_NEAR(third_expected_command, sys.get_input_value("propeller(rpm)"), 1e-6);
}

TEST_F(PIDControllerTest, can_compute_PID_commands_several_times_at_first_time_step)
{
    const double tstart = 0.1;
    const double dt = 0.5;
    ssc::solver::Scheduler scheduler(tstart, 10, dt);

    const double Kp = 2.5;
    const double Ki = 0.1;
    const double Kd = 0.314;
    PIDController controller(
        tstart,
        dt,
        a.random<std::string>(),
        pid_specific_yaml(Kp, Ki, Kd, "rpm_co", "propeller(rpm)") +
            "state weights:\n    x: 1\n    y: -1\n");

    Sim sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);

    // First time step
    ASSERT_EQ(tstart, scheduler.get_time());
    const double error = 0.05;
    const double rpm_co = 5;
    const double x = 2 * rpm_co;
    const double y = rpm_co + error;
    sys.set_command_listener(std::map<std::string, double>({{"t", tstart}, {"rpm_co", rpm_co}}));
    const StateType states = {x, y, 300.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 1, 0, 0, 0};
    sys.get_bodies().front()->update_body_states(states, tstart);
    controller.callback(scheduler, sys);
    const double first_expected_command = Kp * error;
    ASSERT_NEAR(first_expected_command, sys.get_input_value("propeller(rpm)"), 1e-6);

    // First time step again should send back the same result
    ASSERT_EQ(tstart, scheduler.get_time());
    controller.callback(scheduler, sys);
    ASSERT_NEAR(first_expected_command, sys.get_input_value("propeller(rpm)"), 1e-6);

    // A time step before the previous time should send back the same result
    scheduler.add_time_event(tstart - dt / 2);
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(tstart - dt / 2, scheduler.get_time());
    controller.callback(scheduler, sys);
    ASSERT_NEAR(first_expected_command, sys.get_input_value("propeller(rpm)"), 1e-6);
}

TEST_F(PIDControllerTest, can_use_euler_angles_in_states)
{
    const double tstart = 0.1;
    const double dt = 0.5;
    ssc::solver::Scheduler scheduler(tstart, 10, dt);

    const double Kp = 2.5;
    const double Ki = 0.1;
    const double Kd = 0.314;
    PIDController controller(
        tstart,
        dt,
        a.random<std::string>(),
        pid_specific_yaml(Kp, Ki, Kd, "psi", "propeller(psi_co)") + "state weights:\n    psi: 1\n");

    Sim sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);

    const double psi = 0.45;
    const double error = -0.05;
    sys.set_command_listener(std::map<std::string, double>({{"t", tstart}, {"psi", psi}}));

    const ssc::kinematics::EulerAngles angles(0.2, 0.9, psi - error);
    std::tuple<double, double, double, double> quaternion =
        sys.get_bodies().front()->get_quaternions(angles, sys.get_env().rot);
    const StateType states = {
        100.0,
        200.0,
        300.0,
        1.0,
        2.0,
        3.0,
        4.0,
        5.0,
        6.0,
        std::get<0>(quaternion),
        std::get<1>(quaternion),
        std::get<2>(quaternion),
        std::get<3>(quaternion)};
    sys.get_bodies().front()->update_body_states(states, tstart);

    controller.callback(scheduler, sys);
    const double first_expected_command = Kp * error;
    ASSERT_NEAR(first_expected_command, sys.get_input_value("propeller(psi_co)"), 1e-6);
}

TEST_F(CSVControllerTest, can_read_data_from_csv)
{
    std::string yaml = test_yaml();
    boost::replace_all(
        yaml,
        "shift time column to match tstart: true",
        "shift time column to match tstart: false");
    CSVController controller(0, a.random<std::string>(), yaml);
    const double tstart = 0.1;
    const double dt = 0.5;
    ssc::solver::Scheduler scheduler(tstart, 2, dt);
    Sim sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);
    controller.callback(scheduler, sys);
    // First time step is t0 = 0: all commands are zeros because t0 in CSV is 0.2
    ASSERT_NEAR(0, sys.get_input_value("port side propeller(rpm)"), 1e-6);
    ASSERT_NEAR(0, sys.get_input_value("port side propeller(beta)"), 1e-6);
    scheduler.advance_to_next_time_event();
    controller.callback(scheduler, sys);
    // We should now be at t = 0.2 (first line in CSV) because t0+dt = 0 + 0.5 = 0.5 > 0.2
    ASSERT_EQ(0.2, scheduler.get_time());
    // Commands should now contain the first line of the CSV
    ASSERT_NEAR(65, sys.get_input_value("port side propeller(rpm)"), 1e-6);
    ASSERT_NEAR(78, sys.get_input_value("port side propeller(beta)"), 1e-6);
}

TEST_F(CSVControllerTest, can_read_data_from_csv_with_time_shift)
{
    const std::string yaml = test_yaml();
    CSVController controller(0, a.random<std::string>(), yaml);
    const double tstart = 0.1;
    const double dt = 0.5;
    ssc::solver::Scheduler scheduler(tstart, 2, dt);
    Sim sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);
    controller.callback(scheduler, sys);
    // First time step is t0 = 0.1: t0 in CSV was shifted
    ASSERT_EQ(tstart, scheduler.get_time());
    ASSERT_NEAR(65, sys.get_input_value("port side propeller(rpm)"), 1e-6);
    ASSERT_NEAR(78, sys.get_input_value("port side propeller(beta)"), 1e-6);
    // Advance in time
    scheduler.advance_to_next_time_event();
    // Update controller
    controller.callback(scheduler, sys);
    // We should now be at t = 0.6 (first line in CSV) because t0+dt = 0.1 + 0.5 = 0.6
    ASSERT_EQ(0.6, scheduler.get_time());
    // Commands should still contain the first line of the CSV
    ASSERT_NEAR(65, sys.get_input_value("port side propeller(rpm)"), 1e-6);
    ASSERT_NEAR(78, sys.get_input_value("port side propeller(beta)"), 1e-6);
    // Advance in time
    scheduler.advance_to_next_time_event();
    // Update controller
    controller.callback(scheduler, sys);
    // We should now be at t = 0.696 (second line in CSV) because t0+dt = 0.6 + 0.5 = 1.1 > 0.696
    ASSERT_NEAR(0.696, scheduler.get_time(), 1e-6);
    // Commands should now contain the last line of the CSV
    ASSERT_NEAR(965.1, sys.get_input_value("port side propeller(rpm)"), 1e-6);
    ASSERT_NEAR(-7.8, sys.get_input_value("port side propeller(beta)"), 1e-6);
}
