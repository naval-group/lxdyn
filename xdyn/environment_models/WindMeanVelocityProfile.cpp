/*
 * WindMeanVelocityProfile.cpp
 *
 *  Created on: 7 janv. 2020
 *      Author: mcharlou2016
 */

#include "WindMeanVelocityProfile.hpp"
#include "xdyn/yaml_parser/parse_unit_value.hpp"
#include <Eigen/Dense>
#include "xdyn/yaml_parser/yaml_compat.h"


WindMeanVelocityProfile::WindMeanVelocityProfile(const Input& input) : velocity(input.velocity), direction(cos(input.direction), sin(input.direction), 0.)
{
}

Eigen::Vector3d WindMeanVelocityProfile::get_wind(const Eigen::Vector3d& position, const double) const
{
    return get_wind_velocity(position(2)) * direction;
}

WindMeanVelocityProfile::Input WindMeanVelocityProfile::parse(const std::string& yaml_input)
{
    YAML::Node node = YAML::Load(yaml_input);
    Input ret;
    xdyn::yaml_parser::parse_uv(node["velocity"], ret.velocity);
    xdyn::yaml_parser::parse_uv(node["direction"], ret.direction);
    return ret;
}
