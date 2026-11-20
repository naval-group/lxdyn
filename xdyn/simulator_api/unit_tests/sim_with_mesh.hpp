/*
 * sim_with_mesh.hpp
 *
 * For the tests that give their one body its mesh directly rather than through the YAML.
 */

#ifndef SIM_WITH_MESH_HPP_
#define SIM_WITH_MESH_HPP_

#include <optional>
#include <string>

#include "xdyn/mesh/stl_reader.hpp"
#include "xdyn/simulator_api/simulator_api.hpp"
#include "xdyn/yaml_parser/SimulatorYamlParser.hpp"

inline MeshMap mesh_of_first_body(
    const YamlSimulatorInput& input,
    const VectorOfVectorOfPoints& mesh)
{
    return {{input.bodies.front().name, mesh}};
}

inline Sim sim_with_mesh(
    const YamlSimulatorInput& input,
    const VectorOfVectorOfPoints& mesh,
    const double t0 = 0,
    const std::optional<ssc::data_source::DataSource>& commands = std::nullopt)
{
    return get_system(input, mesh_of_first_body(input, mesh), t0, commands);
}

inline Sim sim_with_mesh(
    const std::string& yaml,
    const VectorOfVectorOfPoints& mesh,
    const double t0 = 0,
    const std::optional<ssc::data_source::DataSource>& commands = std::nullopt)
{
    return sim_with_mesh(SimulatorYamlParser(yaml).parse(), mesh, t0, commands);
}

/** \brief `stl` is the STL text, not its path */
inline Sim sim_with_mesh(
    const std::string& yaml,
    const std::string& stl,
    const double t0 = 0,
    const std::optional<ssc::data_source::DataSource>& commands = std::nullopt)
{
    return sim_with_mesh(yaml, read_stl(stl), t0, commands);
}

#endif /* SIM_WITH_MESH_HPP_ */
