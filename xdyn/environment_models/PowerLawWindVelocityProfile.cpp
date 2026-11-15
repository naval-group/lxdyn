/*
 * PowerLawWindVelocityProfile.cpp
 *
 *  Created on: 10 déc. 2020
 *      Author: mcharlou2016
 */

#include "PowerLawWindVelocityProfile.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/yaml_parser/parse_unit_value.hpp"
#include "xdyn/yaml_parser/yaml_compat.h"
#include <cmath>


PowerLawWindVelocityProfile::PowerLawWindVelocityProfile(const Input& input) : WindMeanVelocityProfile(input), alpha(input.alpha), z_ref(input.z_ref)
{
    if(z_ref <= 0)
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "the reference height must be strictly positive");
    }
}

std::string PowerLawWindVelocityProfile::model_name() {return "power law wind profile";}

PowerLawWindVelocityProfile::Input PowerLawWindVelocityProfile::parse(const std::string& yaml_input)
{
    Input ret{WindMeanVelocityProfile::parse(yaml_input)};
    YAML::Node node = YAML::Load(yaml_input);
    node["alpha"] >> ret.alpha;
    xdyn::yaml_parser::parse_uv(node["reference height"], ret.z_ref);
    return ret;
}

double PowerLawWindVelocityProfile::get_wind_velocity(const double z) const
{
    if(z <= 0) return velocity * std::pow(-z / z_ref, alpha); // -z because the NED frame has negative z above sea level
    else return 0.;
}
