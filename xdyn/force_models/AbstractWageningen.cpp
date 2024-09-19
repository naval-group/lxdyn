/*
 * AbstractWageningen.cpp
 *
 *  Created on: Jun 28, 2015
 *      Author: cady
 */
#include "AbstractWageningen.hpp"

#include "xdyn/yaml_parser/external_data_structures_parsers.hpp"

#include "xdyn/yaml_parser/parse_unit_value.hpp"

#include "xdyn/yaml_parser/yaml_compat.h"

#include "ssc/kinematics/RotationMatrix.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

AbstractWageningen::Yaml::Yaml():
    name(),
    position_of_propeller_frame(YamlPosition()),
    wake_coefficient(0.0),
    relative_rotative_efficiency(0.0),
    thrust_deduction_factor(0.0),
    rotating_clockwise(false),
    diameter(0.0)
{}

AbstractWageningen::Yaml AbstractWageningen::parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    Yaml ret;
    std::string rot;
    node["rotation"] >> rot;
    ret.rotating_clockwise = (rot == "clockwise");
    node["thrust deduction factor t"]        >> ret.thrust_deduction_factor;
    node["wake coefficient w"]               >> ret.wake_coefficient;
    node["name"]                             >> ret.name;
    node["position of propeller frame"]      >> ret.position_of_propeller_frame;
    node["relative rotative efficiency etaR"]>> ret.relative_rotative_efficiency;
    xdyn::yaml_parser::parse_uv(node["diameter"], ret.diameter);
    return ret;
}

double AbstractWageningen::get_advance_speed(const BodyStates& states, const double t, const EnvironmentAndFrames& env) const
{
    // The propeller sees the speed of advance through the water, so the NED current has to be
    // expressed in the body frame before it can be taken off the body-frame surge.
    const Eigen::Vector3d current_in_body = states.get_rot_from_ned_to_body().transpose()*env.get_UWCurrent(Eigen::Vector3d(states.x(), states.y(), states.z()), t);
    // Only the surge is scaled by the wake factor, so the transverse components are left out of the
    // rotation into the propeller frame: how the wake affects them is not known.
    const Eigen::Vector3d Va_vector_body_frame((1-w)*(states.u()-current_in_body(0)),0,0);
    // The propeller axis need not be parallel to the hull's, so the advance speed is the projection
    // of that vector onto the propeller frame's X-axis.
    const ssc::kinematics::RotationMatrix R = env.k->get(name,body_name).get_rot();
    return (R*Va_vector_body_frame)(0);
}

double AbstractWageningen::get_advance_ratio(const std::map<std::string,double>& commands, const double Va) const
{
    const double n = commands.at("rpm")/(2*PI);
    return Va/n/D;
}

AbstractWageningen::AbstractWageningen(const Yaml& input, const std::string& body_name_, const EnvironmentAndFrames& env) :
            ForceModel(input.name,{"rpm"},input.position_of_propeller_frame, body_name_, env),
            w(input.wake_coefficient),
            eta_R(input.relative_rotative_efficiency),
            t(input.thrust_deduction_factor),
            kappa(input.rotating_clockwise ? -1 : 1),
            D(input.diameter),
            D4(D*D*D*D),
            D5(D4*D)
{
}

Wrench AbstractWageningen::get_force(const BodyStates& states, const double t_, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands) const
{
    Wrench tau(ssc::kinematics::Point(name,0,0,0), name);
    const double n2 = commands.at("rpm")*commands.at("rpm")/(4*PI*PI); // In turns per second (Hz)
    const double Va = get_advance_speed(states, t_, env);
    const double J = get_advance_ratio(commands, Va);
    // The thrust deduction factor is only meaningful if the propeller axis is parallel to the body X-axis
    tau.X() = (1-t)*env.rho*n2*D4*get_Kt(commands, J);
    tau.K() = kappa*eta_R*env.rho*n2*D5*get_Kq(commands, J);
    return tau;
}
