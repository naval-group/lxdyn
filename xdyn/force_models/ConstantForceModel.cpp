/*
 * ConstantForceModel.cpp
 *
 *  Created on: Sep 7, 2018
 *      Author: cady
 */


#include "ConstantForceModel.hpp"

#include "xdyn/core/Body.hpp"
#include <ssc/macros.hpp>
#include <ssc/kinematics.hpp>
#include "xdyn/yaml_parser/parse_unit_value.hpp"
#include "xdyn/yaml_parser/yaml_compat.h"

std::string ConstantForceModel::model_name() {return "constant force";}

ConstantForceModel::Input ConstantForceModel::parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    ConstantForceModel::Input ret;
    node["frame"] >> ret.frame;
    xdyn::yaml_parser::parse_uv(node["x"], ret.x);
    xdyn::yaml_parser::parse_uv(node["y"], ret.y);
    xdyn::yaml_parser::parse_uv(node["z"], ret.z);
    xdyn::yaml_parser::parse_uv(node["X"], ret.X);
    xdyn::yaml_parser::parse_uv(node["Y"], ret.Y);
    xdyn::yaml_parser::parse_uv(node["Z"], ret.Z);
    xdyn::yaml_parser::parse_uv(node["K"], ret.K);
    xdyn::yaml_parser::parse_uv(node["M"], ret.M);
    xdyn::yaml_parser::parse_uv(node["N"], ret.N);
    return ret;
}

ConstantForceModel::ConstantForceModel(const ConstantForceModel::Input& input, const std::string& body_name_, const EnvironmentAndFrames& env) :
        ForceModel(model_name(), {}, YamlPosition(YamlCoordinates(input.x, input.y, input.z),YamlAngle(),input.frame), body_name_, env),
        force(),
        torque()
{
    force << input.X
           , input.Y
           , input.Z;
    torque << input.K
            , input.M
            , input.N;
}

Wrench ConstantForceModel::get_force(const BodyStates&, const double, const EnvironmentAndFrames&, const std::map<std::string,double>&) const
{
    return Wrench(ssc::kinematics::Point(name,0,0,0), name, force, torque);
}
