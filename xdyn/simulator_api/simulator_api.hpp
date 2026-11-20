#ifndef SIMULATORAPI_HPP_
#define SIMULATORAPI_HPP_

#include <map>
#include <string>

#include "xdyn/core/Sim.hpp"
#include "xdyn/core/SimulatorBuilder.hpp"

struct YamlSimulatorInput;

Sim get_system(const std::string& yaml, const double t0 = 0.0);
Sim get_system(const std::string& yaml, const std::string& mesh, const double t0);
Sim get_system(
    const std::string& yaml,
    const std::map<std::string, VectorOfVectorOfPoints>& meshes,
    const double t0);
Sim get_system(const std::string& yaml, const VectorOfVectorOfPoints& mesh, const double t0);
Sim get_system(const YamlSimulatorInput& input, const double t0 = 0.0);
Sim get_system(const YamlSimulatorInput& input, const std::string& mesh, const double t0);
Sim get_system(
    const YamlSimulatorInput& input,
    const VectorOfVectorOfPoints& mesh,
    const double t0);
Sim get_system(
    const YamlSimulatorInput& input,
    const VectorOfVectorOfPoints& mesh,
    const double t0,
    ssc::data_source::DataSource& commands);
Sim get_system(
    const std::string& yaml,
    const VectorOfVectorOfPoints& mesh,
    const double t0,
    ssc::data_source::DataSource& commands);
Sim get_system(
    const std::string& yaml,
    const std::string& mesh,
    const double t0,
    ssc::data_source::DataSource& commands);
Sim get_system(
    const YamlSimulatorInput& input,
    const std::map<std::string, VectorOfVectorOfPoints>& meshes,
    const double t0);
EnvironmentAndFrames get_environment_for_wave_queries(const std::string& yaml);

#endif
