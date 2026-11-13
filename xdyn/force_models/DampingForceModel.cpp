/*
 * DampingForceModel.cpp
 *
 *  Created on: Oct 17, 2014
 *      Author: cady
 */

#include "DampingForceModel.hpp"

#include <Eigen/Dense>

#include "xdyn/core/Body.hpp"

DampingForceModel::DampingForceModel(
    const std::string& name_,
    const std::string& body_name_,
    const EnvironmentAndFrames& env,
    const Eigen::Matrix<double, 6, 6>& D_)
    : ForceModel(name_, {}, body_name_, env), D(D_)
{
}

Wrench DampingForceModel::get_force(
    const BodyStates& states,
    const double t,
    const EnvironmentAndFrames& env,
    const std::map<std::string, double>& /*commands*/) const
{
    // Damping acts on the velocity relative to the water, so the NED current has to be expressed in
    // the body frame before it can be taken off the body-frame velocities.
    const Eigen::Vector3d current_in_body =
        states.get_rot_from_ned_to_body().transpose() *
        env.get_UWCurrent(Eigen::Vector3d(states.x(), states.y(), states.z()), t);
    Eigen::Matrix<double, 6, 1> W;
    W << states.u() - current_in_body(0), states.v() - current_in_body(1),
        states.w() - current_in_body(2), states.p(), states.q(), states.r();
    return Wrench(
        states.hydrodynamic_forces_calculation_point,
        body_name,
        get_force_and_torque(D, W));
}
