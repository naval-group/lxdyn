/*
 * build_controllers.cpp
 */

#include "build_controllers.hpp"

#include <boost/algorithm/string.hpp>
#include <ssc/solver/Scheduler.hpp>

#include "xdyn/controllers/Controller.hpp"
#include "xdyn/core/Sim.hpp"
#include "xdyn/simulator_api/model_tables.hpp"
#include "xdyn/yaml_parser/check_input_yaml.hpp"

std::vector<std::shared_ptr<ssc::solver::DiscreteSystem>> build_controllers(
    const double tstart,
    const std::vector<YamlController>& yaml_controllers,
    const std::vector<YamlTimeSeries>& yaml_commands,
    Sim& sys)
{
    std::vector<std::shared_ptr<ssc::solver::DiscreteSystem>> controllers;
    for (YamlController yaml_controller : yaml_controllers) {
        boost::to_upper(yaml_controller.type);
        const auto controller = make_controller(tstart, yaml_controller, sys);
        for (const auto& command_name : controller->get_command_names()) {
            check_controller_output_is_not_defined_in_a_command(command_name, yaml_commands);
        }
        controllers.push_back(controller);
    }
    return controllers;
}

std::vector<std::shared_ptr<ssc::solver::DiscreteSystem>> get_initialized_controllers(
    const double tstart,
    const std::vector<YamlController>& yaml_controllers,
    const std::vector<YamlTimeSeries>& yaml_commands,
    ssc::solver::Scheduler& scheduler,
    Sim& sys)
{
    const auto controllers = build_controllers(tstart, yaml_controllers, yaml_commands, sys);
    sys.set_discrete_state("t", scheduler.get_time());
    for (const auto& controller : controllers) {
        controller->initialize(scheduler, sys);
    }
    return controllers;
}
