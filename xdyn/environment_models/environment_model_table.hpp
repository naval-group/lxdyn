/*
 * environment_model_table.hpp
 */

#ifndef ENVIRONMENT_MODEL_TABLE_HPP_
#define ENVIRONMENT_MODEL_TABLE_HPP_

#include <string>
#include <vector>

#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/external_data_structures/YamlModel.hpp"

/** \brief Builds the model and fills its slot of `env`: the waves, the wind or the current */
void add_environment_model(const YamlModel& yaml, EnvironmentAndFrames& env);

std::vector<std::string> environment_model_names();

#endif /* ENVIRONMENT_MODEL_TABLE_HPP_ */
