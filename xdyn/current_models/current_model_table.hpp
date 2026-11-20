/*
 * current_model_table.hpp
 */

#ifndef CURRENT_MODEL_TABLE_HPP_
#define CURRENT_MODEL_TABLE_HPP_

#include <string>
#include <vector>

#include "xdyn/current_models/UWCurrentModel.hpp"

using UWCurrentModelFactory = UWCurrentModelPtr (*)(const std::string& yaml);

/** \brief The factory of the current model named `name`, nullptr when none has that name */
UWCurrentModelFactory find_current_model(const std::string& name);

std::vector<std::string> current_model_names();

#endif /* CURRENT_MODEL_TABLE_HPP_ */
