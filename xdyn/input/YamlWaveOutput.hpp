/*
 * YamlWaveOutput.hpp
 *
 *  Created on: May 27, 2019
 *      Author: cady
 */

#ifndef EXTERNAL_DATA_STRUCTURES_INC_YAMLWAVEOUTPUT_HPP_
#define EXTERNAL_DATA_STRUCTURES_INC_YAMLWAVEOUTPUT_HPP_

#include <string>

struct YamlWaveOutput {
    YamlWaveOutput();
    /// Name of the frame of reference the x & y coordinates are expressed in
    std::string frame_of_reference;
    /// Minimum x value (in meters) of the points in the output mesh
    double xmin{0};
    /// Maximum x value (in meters) of the points in the output mesh
    double xmax{0};
    /// Discretization of the mesh along the x axis (number of different x values)
    size_t nx{0};
    /// Minimum y value (in meters) of the points in the output mesh
    double ymin{0};
    /// Maximum y value (in meters) of the points in the output mesh
    double ymax{0};
    /// Discretization of the mesh along the y axis (number of different x values)
    size_t ny{0};
};

#endif /* EXTERNAL_DATA_STRUCTURES_INC_YAMLWAVEOUTPUT_HPP_ */
