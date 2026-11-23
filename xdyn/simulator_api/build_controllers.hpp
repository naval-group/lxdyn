/*
 * build_controllers.hpp
 */

#ifndef BUILD_CONTROLLERS_HPP_
#define BUILD_CONTROLLERS_HPP_

#include <memory>
#include <vector>

#include "xdyn/input/YamlController.hpp"
#include "xdyn/input/YamlTimeSeries.hpp"

class Sim;
namespace ssc::solver {
class DiscreteSystem;
class Scheduler;
}  // namespace ssc::solver

/**  \brief Builds the controllers `yaml_controllers` declares
 *  \snippet simulator_api/unit_tests/build_controllersTest.cpp build_controllers_example
 */
std::vector<std::shared_ptr<ssc::solver::DiscreteSystem>> build_controllers(
    const double tstart,
    const std::vector<YamlController>& yaml_controllers,
    const std::vector<YamlTimeSeries>& yaml_commands,
    Sim& sys);

/**  \brief Builds the controllers, sets their first commands in `sys` and schedules their callbacks
 *  \snippet simulator_api/unit_tests/build_controllersTest.cpp initialized_controllers_example
 */
std::vector<std::shared_ptr<ssc::solver::DiscreteSystem>> get_initialized_controllers(
    const double tstart,
    const std::vector<YamlController>& yaml_controllers,
    const std::vector<YamlTimeSeries>& yaml_commands,
    ssc::solver::Scheduler& scheduler,
    Sim& sys);

#endif /* BUILD_CONTROLLERS_HPP_ */
