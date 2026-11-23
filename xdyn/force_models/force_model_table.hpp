/*
 * force_model_table.hpp
 */

#ifndef FORCE_MODEL_TABLE_HPP_
#define FORCE_MODEL_TABLE_HPP_

#include <string>
#include <vector>

#include "xdyn/core/ForceModel.hpp"

using ForceModelFactory = ForcePtr (*)(
    const std::string& yaml,
    const std::string& body_name,
    const EnvironmentAndFrames& env);

/** \brief The factory of the force model named `name`, nullptr when none has that name */
ForceModelFactory find_force_model(const std::string& name);

std::vector<std::string> force_model_names();

#endif /* FORCE_MODEL_TABLE_HPP_ */
