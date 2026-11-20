/*
 * wave_model_table.hpp
 */

#ifndef WAVE_MODEL_TABLE_HPP_
#define WAVE_MODEL_TABLE_HPP_

#include <string>
#include <vector>

#include "xdyn/wave_models/SurfaceElevationInterface.hpp"

using SurfaceElevationFactory = SurfaceElevationPtr (*)(const std::string& yaml);

/** \brief The factory of the free surface named `name`, nullptr when none has that name */
SurfaceElevationFactory find_surface_elevation(const std::string& name);

std::vector<std::string> surface_elevation_names();

#endif /* WAVE_MODEL_TABLE_HPP_ */
