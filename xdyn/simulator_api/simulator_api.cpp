#include "simulator_api.hpp"

#include "xdyn/controllers/listeners.hpp"
#include "xdyn/core/SimulatorBuilder.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/simulator_api/model_tables.hpp"
#include "xdyn/yaml_parser/SimulatorYamlParser.hpp"
#include "xdyn/yaml_parser/check_input_yaml.hpp"

namespace {
SimulatorBuilder get_builder(
    const YamlSimulatorInput& input,
    const double t0,
    const ssc::data_source::DataSource& commands = ssc::data_source::DataSource())
{
    return SimulatorBuilder(input, make_force_model, add_environment_model, t0, commands);
}

ssc::data_source::DataSource commands_and_setpoints(const YamlSimulatorInput& input)
{
    ssc::data_source::DataSource commands = make_command_listener(input.commands);
    add_setpoints_listener(commands, input.setpoints);
    return commands;
}
}  // namespace

Sim get_system(const YamlSimulatorInput& input, const double t0)
{
    check_input_yaml(input);
    return get_builder(input, t0, commands_and_setpoints(input)).build();
}

Sim get_system(
    const YamlSimulatorInput& input,
    const MeshMap& meshes,
    const double t0,
    const std::optional<ssc::data_source::DataSource>& commands)
{
    check_input_yaml(input);
    return get_builder(input, t0, commands ? *commands : commands_and_setpoints(input))
        .build(meshes);
}

Sim get_system(const std::string& yaml, const double t0)
{
    return get_system(SimulatorYamlParser(yaml).parse(), t0);
}

EnvironmentAndFrames get_environment_for_wave_queries(const std::string& yaml_data)
{
    const YamlSimulatorInput input = check_input_yaml(SimulatorYamlParser(yaml_data).parse());
    EnvironmentAndFrames env = get_builder(input, 0.0).build_environment_and_frames();
    if (env.w == nullptr) {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "No wave environment model is defined");
    }
    if (env.g == 0.0) {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "Gravity constant g can not be zero");
    }
    if (env.rho == 0.0) {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "Water density rho can not be zero");
    }
    return env;
}
