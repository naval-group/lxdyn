/*
 * GZTypes.hpp
 *
 *  Created on: Feb 9, 2015
 *      Author: cady
 */

#ifndef GZTYPES_HPP_
#define GZTYPES_HPP_

#include <Eigen/Dense>

namespace GZ
{
    using State = Eigen::Vector3d;

    struct Resultant
    {
        Resultant();
        State state;
        double gz;
    };
}

#endif /* GZTYPES_HPP_ */
