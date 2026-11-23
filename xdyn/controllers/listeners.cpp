/*
 * listeners.cpp
 *
 *  Created on: Oct 21, 2014
 *      Author: cady
 */

#include "listeners.hpp"

#include <memory>
#include <ssc/macros.hpp>

#include "InterpolationModule.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/input/YamlTimeSeries.hpp"
#include "xdyn/yaml_parser/check_input_yaml.hpp"

void add_interpolation_table(
    const std::string& x_name,
    const std::vector<double>& x,
    const std::string& y_name,
    const std::vector<double>& y,
    ssc::data_source::DataSource& ds);
void add_interpolation_table(
    const std::string& x_name,
    const std::vector<double>& x,
    const std::string& y_name,
    const std::vector<double>& y,
    ssc::data_source::DataSource& ds)
{
    ds.check_in(__PRETTY_FUNCTION__);
    std::shared_ptr<ssc::interpolation::LinearInterpolationVariableStep> I =
        std::make_shared<ssc::interpolation::LinearInterpolationVariableStep>(x, y);

    const std::string module_name = x_name + "->" + y_name;
    InterpolationModule module(&ds, module_name, x_name, y_name, I);
    ds.add(module);
    ds.check_out();
}

std::string namify(const std::string& command_name, const std::string& model_name);
std::string namify(const std::string& command_name, const std::string& model_name)
{
    if (model_name == "") {
        return command_name;
    }
    return model_name + "(" + command_name + ")";
}

void add(
    std::vector<YamlTimeSeries>::const_iterator& that_command,
    ssc::data_source::DataSource& ds);
void add(
    std::vector<YamlTimeSeries>::const_iterator& that_command,
    ssc::data_source::DataSource& ds)
{
    ds.check_in(__PRETTY_FUNCTION__);
    const auto t = that_command->t;
    if (t.size() == 1) {
        for (auto it = that_command->values.begin(); it != that_command->values.end(); ++it) {
            ds.set<double>(namify(it->first, that_command->name), it->second.front());
        }
    } else {
        for (auto it = that_command->values.begin(); it != that_command->values.end(); ++it) {
            try {
                add_interpolation_table(
                    "t",
                    t,
                    namify(it->first, that_command->name),
                    it->second,
                    ds);
            } catch (const ssc::interpolation::PiecewiseConstantVariableStepException& e) {
                THROW(
                    __PRETTY_FUNCTION__,
                    InvalidInputException,
                    "Unable to build interpolation table between 't' and '"
                        << it->first << "' for force model '" << that_command->name
                        << "': " << e.get_message());
            }
        }
    }
    ds.check_out();
}

ssc::data_source::DataSource make_command_listener(
    const std::vector<YamlTimeSeries>& commands  //!< Parsed YAML commands
)
{
    check_command_names(commands);
    ssc::data_source::DataSource ds;
    ds.check_in(__PRETTY_FUNCTION__);
    for (auto that_command = commands.begin(); that_command != commands.end(); ++that_command) {
        add(that_command, ds);
    }
    ds.check_out();
    return ds;
}

void add_setpoints_listener(
    ssc::data_source::DataSource& ds,
    const std::vector<YamlTimeSeries>& setpoints  //!< Parsed YAML setpoints
)
{
    ds.check_in(__PRETTY_FUNCTION__);
    for (auto that_setpoint = setpoints.begin(); that_setpoint != setpoints.end();
         ++that_setpoint) {
        add(that_setpoint, ds);
    }
    ds.check_out();
}
