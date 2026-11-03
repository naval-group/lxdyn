/*
 * coriolis_and_centripetal.cpp
 *
 * \date Jun 3, 2014
 * \author cec
 */


#include "ssc/kinematics/coriolis_and_centripetal.hpp"
#include "ssc/kinematics/Point.hpp"

using namespace Eigen;
using namespace ssc::kinematics;

Wrench ssc::kinematics::coriolis_and_centripetal(const Point& P, const Matrix6x6& M, const Vector3d& v, const Vector3d& omega)
{
    const Matrix3x3 A11 = M.block<3,3>(0,0);
    const Matrix3x3 A12 = M.block<3,3>(0,3);
    const Matrix3x3 A21 = M.block<3,3>(3,0);
    const Matrix3x3 A22 = M.block<3,3>(3,3);

    const Vector3d s1 = A11*v+A12*omega;
    const Vector3d s2 = A21*v+A22*omega;

    const Vector3d force = s1.cross(omega);
    const Vector3d torque = s1.cross(v)+s2.cross(omega);

    return Wrench(P, force, torque);
}
