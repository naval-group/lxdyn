/*
 * ConstantUWCurrentModel.cpp
 */

#include "ConstantUWCurrentModel.hpp"

#include <cmath>
#include "xdyn/yaml_parser/yaml_compat.h"
#include "xdyn/yaml_parser/parse_unit_value.hpp"

ConstantUWCurrentModel::ConstantUWCurrentModel(const Input& input) :
    velocity(input.velocity),
    orientation(input.orientation)
{
}

std::string ConstantUWCurrentModel::model_name() {return "constant UW current";}

ConstantUWCurrentModel::Input ConstantUWCurrentModel::parse(const std::string& yaml_input)
{
    YAML::Node node = YAML::Load(yaml_input);
    Input result;
    xdyn::yaml_parser::parse_uv(node["velocity"], result.velocity);
    xdyn::yaml_parser::parse_uv(node["orientation"], result.orientation);
    return result;
}

Eigen::Vector3d ConstantUWCurrentModel::get_UWCurrent(const Eigen::Vector3d& position, const double, const double wave_height) const
{
    if (position.z() > wave_height)
    {
        return Eigen::Vector3d(velocity*std::cos(orientation), velocity*std::sin(orientation), 0.);
    }
    return Eigen::Vector3d::Zero();
}
