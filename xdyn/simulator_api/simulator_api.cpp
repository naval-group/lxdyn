#include "simulator_api.hpp"

#include "xdyn/controllers/listeners.hpp"
#include "xdyn/core/SimulatorBuilder.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/mesh/stl_reader.hpp"
#include "xdyn/simulator_api/model_tables.hpp"
#include "xdyn/yaml_parser/SimulatorYamlParser.hpp"
#include "xdyn/yaml_parser/check_input_yaml.hpp"

SimulatorBuilder get_builder(
    const YamlSimulatorInput& input,
    const double t0,
    const ssc::data_source::DataSource& command_listener = ssc::data_source::DataSource());
SimulatorBuilder get_builder(
    const YamlSimulatorInput& input,
    const double t0,
    const ssc::data_source::DataSource& command_listener)
{
    return SimulatorBuilder(input, make_force_model, add_environment_model, t0, command_listener);
}

Sim get_system(const YamlSimulatorInput& input, const double t0)
{
    check_input_yaml(input);
    ssc::data_source::DataSource command_listener = make_command_listener(input.commands);
    add_setpoints_listener(command_listener, input.setpoints);
    return get_builder(input, t0, command_listener).build();
}

Sim get_system(const YamlSimulatorInput& input, const std::string& mesh, const double t0)
{
    const auto name = input.bodies.front().name;
    MeshMap meshes;
    meshes[name] = read_stl(mesh);
    return get_system(input, meshes, t0);
}

Sim get_system(
    const std::string& yaml,
    const std::string& mesh,
    const double t0,
    ssc::data_source::DataSource& commands)
{
    return get_system(yaml, read_stl(mesh), t0, commands);
}

Sim get_system(const YamlSimulatorInput& input, const VectorOfVectorOfPoints& mesh, const double t0)
{
    const auto name = input.bodies.front().name;
    MeshMap meshes;
    meshes[name] = mesh;
    return get_system(input, meshes, t0);
}

Sim get_system(
    const std::string& yaml,
    const VectorOfVectorOfPoints& mesh,
    const double t0,
    ssc::data_source::DataSource& commands)
{
    return get_system(SimulatorYamlParser(yaml).parse(), mesh, t0, commands);
}

Sim get_system(
    const YamlSimulatorInput& input,
    const VectorOfVectorOfPoints& mesh,
    const double t0,
    ssc::data_source::DataSource& command_listener)
{
    const auto name = input.bodies.empty() ? "" : input.bodies.front().name;
    MeshMap meshes;
    meshes[name] = mesh;
    return get_builder(input, t0, command_listener).build(meshes);
}

Sim get_system(const std::string& yaml, const VectorOfVectorOfPoints& mesh, const double t0)
{
    const auto input = SimulatorYamlParser(yaml).parse();
    const auto name = input.bodies.empty() ? "" : input.bodies.front().name;
    MeshMap meshes;
    meshes[name] = mesh;
    return get_system(input, meshes, t0);
}

Sim get_system(const YamlSimulatorInput& input, const MeshMap& meshes, const double t0)
{
    ssc::data_source::DataSource command_listener = make_command_listener(input.commands);
    add_setpoints_listener(command_listener, input.setpoints);
    return get_builder(input, t0, command_listener).build(meshes);
}

Sim get_system(const std::string& yaml, const double t0)
{
    const auto input = SimulatorYamlParser(yaml).parse();
    return get_system(input, t0);
}

Sim get_system(const std::string& yaml, const std::string& mesh, const double t0)
{
    const auto input = SimulatorYamlParser(yaml).parse();
    return get_system(check_input_yaml(input), mesh, t0);
}

Sim get_system(const std::string& yaml, const MeshMap& meshes, const double t0)
{
    const auto input = SimulatorYamlParser(yaml).parse();
    return get_system(check_input_yaml(input), meshes, t0);
}

EnvironmentAndFrames get_environment_for_wave_queries(const std::string& yaml_data)
{
    EnvironmentAndFrames env(get_system(yaml_data, 0.0).get_env());
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
