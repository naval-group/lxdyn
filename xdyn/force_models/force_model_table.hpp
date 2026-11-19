/*
 * force_model_table.hpp
 */

#ifndef FORCE_MODEL_TABLE_HPP_
#define FORCE_MODEL_TABLE_HPP_

#include <string>
#include <vector>

#include "xdyn/core/ForceModel.hpp"

ForcePtr make_force_model(
    const YamlModel& yaml,
    const std::string& body_name,
    const EnvironmentAndFrames& env);

std::vector<std::string> force_model_names();

#endif /* FORCE_MODEL_TABLE_HPP_ */
