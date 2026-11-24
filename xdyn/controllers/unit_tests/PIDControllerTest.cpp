/*
 * PIDControllerTest.cpp
 *
 *  Created on: Fev 12, 2021
 *      Author: lincker
 */

#include "PIDControllerTest.hpp"

#include <cmath>
#include <ssc/solver/DiscreteSystem.hpp>

#include "xdyn/controllers/PIDController.hpp"
#include "xdyn/yaml_parser/parse_controllers.hpp"
#define PI M_PI

PIDControllerTest::PIDControllerTest() : a(ssc::random_data_generator::DataGenerator(54545)) {}

std::string PIDControllerTest::pid_specific_yaml(
    const double Kp,
    const double Ki,
    const double Kd,
    const std::string& setpoint_name,
    const std::string& command_name)
{
    std::stringstream yaml_string;
    yaml_string << "gains:\n"
                << "    Kp: " << std::to_string(Kp) << "\n"
                << "    Ki: " << std::to_string(Ki) << "\n"
                << "    Kd: " << std::to_string(Kd) << "\n"
                << "setpoint: " << setpoint_name << "\n"
                << "command: " << command_name << "\n";
    return yaml_string.str();
}

TEST_F(PIDControllerTest, can_parse_controller_specific_yaml)
{
    for (size_t i = 0; i < 100; ++i) {
        const double dt = 0.2;
        const double Kp = a.random<double>();
        const double Ki = a.random<double>();
        const double Kd = a.random<double>();
        const std::string setpoint_name = a.random<std::string>();
        const std::string command_name = a.random<std::string>();

        const double x_weight = a.random<double>();
        const double psi_weight = a.random<double>();
        const PIDController controller = PIDController(
            0,
            dt,
            a.random<std::string>(),
            pid_specific_yaml(Kp, Ki, Kd, setpoint_name, command_name) + "state weights:\n    x: " +
                std::to_string(x_weight) + "\n    psi: " + std::to_string(psi_weight) + "\n");

        ASSERT_NEAR(Kp, controller.yaml.Kp, 1e-6);
        ASSERT_NEAR(Ki, controller.yaml.Ki, 1e-6);
        ASSERT_NEAR(Kd, controller.yaml.Kd, 1e-6);
        std::map<std::string, double> state_weights = controller.yaml.state_weights;
        ASSERT_EQ(2, state_weights.size());
        ASSERT_EQ(1, state_weights.count("x"));
        ASSERT_NEAR(x_weight, state_weights["x"], 1e-6);
        ASSERT_EQ(1, state_weights.count("psi"));
        ASSERT_NEAR(psi_weight, state_weights["psi"], 1e-6);
        ASSERT_EQ(setpoint_name, controller.yaml.setpoint_name);
        ASSERT_EQ(command_name, controller.yaml.command_name);
    }
}

TEST_F(PIDControllerTest, can_get_the_commands_outputted_by_the_controller)
{
    const PIDController controller(
        a.random<double>(),
        a.random<double>(),
        a.random<std::string>(),
        pid_specific_yaml(
            a.random<double>(),
            a.random<double>(),
            a.random<double>(),
            "psi",
            "propeller(psi_co)") +
            "state weights:\n    psi: 1\n");
    ASSERT_EQ(1, controller.get_command_names().size());
    ASSERT_EQ("propeller(psi_co)", controller.get_command_names().at(0));
}
