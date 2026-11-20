#ifndef SIMULATORAPI_HPP_
#define SIMULATORAPI_HPP_

#include <optional>
#include <string>

#include "xdyn/core/Sim.hpp"
#include "xdyn/core/SimulatorBuilder.hpp"

struct YamlSimulatorInput;

// Every get_system runs check_input_yaml on its input before building it.

/** \brief Builds each body with the mesh its YAML names, and the commands and setpoints its YAML
 * declares */
Sim get_system(const YamlSimulatorInput& input, const double t0 = 0.0);

/** \brief Builds each body with its mesh in `meshes`, and with `commands` in place of the YAML's
 * when given */
Sim get_system(
    const YamlSimulatorInput& input,
    const MeshMap& meshes,
    const double t0,
    const std::optional<ssc::data_source::DataSource>& commands = std::nullopt);

Sim get_system(const std::string& yaml, const double t0 = 0.0);

EnvironmentAndFrames get_environment_for_wave_queries(const std::string& yaml);

#endif
