/*
 * read_added_mass.hpp
 */

#ifndef READ_ADDED_MASS_HPP_
#define READ_ADDED_MASS_HPP_

#include "xdyn/input/YamlDynamics6x6Matrix.hpp"

/** \brief `added_mass` as six rows, read from the HDB or Precal file it names if it names one */
YamlDynamics6x6Matrix read_added_mass(const YamlDynamics6x6Matrix& added_mass);

#endif /* READ_ADDED_MASS_HPP_ */
