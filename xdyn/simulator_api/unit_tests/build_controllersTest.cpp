/*
 * build_controllersTest.cpp
 */

#include <gtest/gtest.h>

#include <ssc/solver/Scheduler.hpp>

#include "xdyn/controllers/PIDController.hpp"
#include "xdyn/controllers/controller_table.hpp"
#include "xdyn/core/Sim.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/simulator_api/build_controllers.hpp"
#include "xdyn/simulator_api/simulator_api.hpp"
#include "xdyn/yaml_parser/parse_controllers.hpp"
#include "xdyn/yaml_parser/parse_time_series.hpp"

namespace {
Sim get_dummy_sim()
{
    const std::vector<BodyPtr> bodies;
    const std::vector<ListOfForces> forces;
    const EnvironmentAndFrames env;
    const StateType x;
    const ssc::data_source::DataSource command_listener;
    return Sim(bodies, forces, env, x, command_listener);
}
}  // namespace

TEST(build_controllers, builds_the_pid_controllers_the_yaml_declares)
{
    const std::string commands = "commands:\n"
                                 "  - name: propeller\n"
                                 "    t: [0]\n"
                                 "    rpm_co: {unit: rad/s, values: [3]}\n"
                                 "  - name: controller\n"
                                 "    t: [0]\n"
                                 "    rpm: {unit: rad/s, values: [3]}\n"
                                 "    psi_co: {unit: deg, values: [30]}\n";

    Sim sim = get_dummy_sim();
    //! [build_controllers_example]
    const auto controllers = build_controllers(
        0,
        parse_controller_yaml(test_data::read("yaml/controllers.yml")),
        parse_command_yaml(commands),
        sim);
    ASSERT_EQ(2, controllers.size());
    ASSERT_EQ("propeller(rpm)", ((PIDController*)controllers.at(0).get())->yaml.command_name);
    ASSERT_EQ("controller(psi)", ((PIDController*)controllers.at(1).get())->yaml.command_name);
    //! [build_controllers_example]
}

TEST(build_controllers, a_controller_output_may_not_be_a_command)
{
    const std::vector<YamlTimeSeries> yaml_commands = parse_command_yaml(
        "commands:\n"
        "  - name: controller\n"
        "    t: [0]\n"
        "    psi_co: {unit: deg, values: [30]}\n"
        "  - name: propeller\n"
        "    t: [0]\n"
        "    rpm: {unit: rad/s, values: [3]}\n");
    const std::vector<YamlController> yaml_controllers =
        parse_controller_yaml(test_data::read("yaml/controllers.yml"));

    Sim sim = get_dummy_sim();
    ASSERT_THROW(build_controllers(0, yaml_controllers, yaml_commands, sim), InvalidInputException);
}

TEST(get_initialized_controllers, sets_the_first_commands_and_schedules_the_callbacks)
{
    //! [initialized_controllers_example]
    Sim sys = get_system(test_data::read("tutorials/tutorial_01_falling_ball.yml"), 0);
    const double tstart = 0.1;
    const double dt = 1;
    ssc::solver::Scheduler scheduler(tstart, 10, dt);

    // initialize setpoints
    const double rpm_co = 0.45;
    const double pd_co = 0.5;
    sys.set_command_listener(
        std::map<std::string, double>({{"t", tstart}, {"rpm_co", rpm_co}, {"P/D_co", pd_co}}));

    // initialize states
    const double u = 1.0;
    const StateType states = {100.0, 200.0, 300.0, u, 2.0, 3.0, 4.0, 5.0, 6.0, 1, 0, 0, 0};
    sys.get_bodies().front()->update_body_states(states, tstart);

    std::stringstream yaml_controllers;
    yaml_controllers << "controllers:\n"
                        "  - type: PID\n"
                        "    dt: 0.15\n"
                        "    setpoint: rpm_co\n"
                        "    name: foo\n"
                        "    state weights:\n"
                        "        u: 0.5\n"
                        "    command: propeller(rpm)\n"
                        "    gains:\n"
                        "        Kp: 4.2\n"
                        "        Ki: 0.25\n"
                        "        Kd: 1\n"
                        "  - type: PID\n"
                        "    dt: 0.5\n"
                        "    setpoint: P/D_co\n"
                        "    name: bar\n"
                        "    state weights:\n"
                        "        u: 0\n"
                        "    command: controller(P/D)\n"
                        "    gains:\n"
                        "        Kp: 1\n"
                        "        Ki: 0\n"
                        "        Kd: 0\n";

    const auto controllers = get_initialized_controllers(
        tstart,
        parse_controller_yaml(yaml_controllers.str()),
        std::vector<YamlTimeSeries>(),
        scheduler,
        sys);

    // Check controllers commands have been initialized in the datasource
    ASSERT_EQ(tstart, scheduler.get_time());
    ASSERT_NEAR(4.2 * (rpm_co - u * 0.5), sys.get_input_value("propeller(rpm)"), 1e-6);
    ASSERT_NEAR(pd_co, sys.get_input_value("controller(P/D)"), 1e-6);
    // no controller callback should be called anymore at t=tstart
    ASSERT_EQ(0, scheduler.get_discrete_state_updaters_to_run().size());

    // Check 'propeller' callback has been added to the scheduler.
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(tstart + 0.15, scheduler.get_time());
    ASSERT_EQ(1, scheduler.get_discrete_state_updaters_to_run().size());  // propeller callback

    // Check 'propeller' callback has been added to the scheduler.
    scheduler.advance_to_next_time_event();
    ASSERT_EQ(tstart + 0.5, scheduler.get_time());
    ASSERT_EQ(1, scheduler.get_discrete_state_updaters_to_run().size());  // controller callback

    //! [initialized_controllers_example]
}

TEST(build_controllers, an_unknown_type_suggests_the_closest_known_one)
{
    YamlController yaml;
    yaml.type = "PIF";
    yaml.name = "foo";
    Sim sim = get_dummy_sim();
    try {
        build_controllers(0, {yaml}, {}, sim);
        FAIL() << "PIF is no controller type";
    } catch (const InvalidInputException& e) {
        EXPECT_NE(std::string::npos, e.get_message().find("Did you mean 'PID'?"))
            << e.get_message();
    }
}

// make_controller asks controllers/' table first, which would hide a gRPC type of the same name
TEST(build_controllers, the_grpc_type_is_free_in_the_controller_table)
{
    EXPECT_EQ(nullptr, find_controller("GRPC"));
}
