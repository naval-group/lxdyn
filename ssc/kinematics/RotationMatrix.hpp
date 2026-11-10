/*
 * RotationMatrix.hpp
 *
 * \date 8 avr. 2014
 * \author cec
 */

#ifndef ROTATIONMATRIX_HPP_
#define ROTATIONMATRIX_HPP_

#include <Eigen/Dense>

namespace ssc
{
    namespace kinematics
    {
        using RotationMatrix = Eigen::Matrix<double, 3, 3>;
    }
}

#endif /* ROTATIONMATRIX_HPP_ */
