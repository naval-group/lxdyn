/*
 * wind_model_table.hpp
 */

#ifndef WIND_MODEL_TABLE_HPP_
#define WIND_MODEL_TABLE_HPP_

#include <string>
#include <vector>

#include "xdyn/wind_models/WindModel.hpp"

using WindModelFactory = WindModelPtr (*)(const std::string& yaml);

/** \brief The factory of the wind model named `name`, nullptr when none has that name */
WindModelFactory find_wind_model(const std::string& name);

std::vector<std::string> wind_model_names();

#endif /* WIND_MODEL_TABLE_HPP_ */
