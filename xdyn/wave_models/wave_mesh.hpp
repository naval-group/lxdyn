/*
 * wave_mesh.hpp
 */

#ifndef WAVE_MESH_HPP_
#define WAVE_MESH_HPP_

#include <ssc/kinematics.hpp>
#include <utility>

struct YamlWaveOutput;

/** \brief The points at which the wave elevation is written out */
ssc::kinematics::PointMatrixPtr make_wave_mesh(const YamlWaveOutput& output);

/** \brief (nx, ny) of make_wave_mesh, 1 along an axis whose extent is zero */
std::pair<std::size_t, std::size_t> wave_mesh_size(const YamlWaveOutput& output);

#endif /* WAVE_MESH_HPP_ */
