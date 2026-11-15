/*
 * LogWindVelocityProfile.cpp
 *
 *  Created on: 11 déc. 2020
 *      Author: mcharlou2016
 */
#include "LogWindVelocityProfile.hpp"
#include "xdyn/yaml_parser/parse_unit_value.hpp"
#include "xdyn/yaml_parser/yaml_compat.h"
#include <cmath>


LogWindVelocityProfile::LogWindVelocityProfile(const Input& input) : WindMeanVelocityProfile(input), z0(input.z0), z_ref(input.z_ref)
{
    if(z_ref <= 0)
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "the reference height must be strictly positive");
    }
    if(z0 <= 0)
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "the roughness length must be strictly positive");
    }
}

std::string LogWindVelocityProfile::model_name() {return "log wind profile";}

LogWindVelocityProfile::Input LogWindVelocityProfile::parse(const std::string& yaml_input)
{
    Input ret{WindMeanVelocityProfile::parse(yaml_input)};
    YAML::Node node = YAML::Load(yaml_input);
    xdyn::yaml_parser::parse_uv(node["roughness length"], ret.z0);
    xdyn::yaml_parser::parse_uv(node["reference height"], ret.z_ref);
    return ret;
}

double LogWindVelocityProfile::get_wind_velocity(const double z) const
{
    if(z <= 0) return velocity * std::log(-z / z0) / std::log(z_ref / z0); // -z because the NED frame has negative z above sea level
    else return 0.;
}
