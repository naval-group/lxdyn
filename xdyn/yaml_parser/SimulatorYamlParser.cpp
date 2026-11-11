/*
 * SimulatorYamlParser.cpp
 *
 *  Created on: 15 avr. 2014
 *      Author: cady
 */

#include "SimulatorYamlParser.hpp"
#include "external_data_structures_parsers.hpp"
#include "parse_controllers.hpp"
#include "parse_time_series.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "yaml_compat.h"

SimulatorYamlParser::SimulatorYamlParser(const std::string& data) : contents(data)
{
}

#define PARSE_OPTIONAL_KEY(key, dest) \
if(node[key])\
    {\
        node[key] >> (dest);\
    }


#define CHECK_KEY_EXISTS(key) \
    if (!node[key]) \
    { \
        THROW(__PRETTY_FUNCTION__, InvalidInputException, \
            std::string("YAML node \"") + (key) + \
            std::string("\" is missing in the input data. Please add this node. See xdyn help!")); \
    }

YamlSimulatorInput SimulatorYamlParser::parse() const
{
    YAML::Node node = YAML::Load(contents);
    if (node.size() == 0)
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException,
            "Something is wrong with the YAML data: no YAML nodes were detected by the YAML parser.");
    }
    CHECK_KEY_EXISTS("rotations convention");
    CHECK_KEY_EXISTS("environmental constants");
    CHECK_KEY_EXISTS("environment models");
    YamlSimulatorInput ret;
    node["environmental constants"] >> ret.environmental_constants;
    node["rotations convention"] >> ret.rotations;
    node["environment models"] >> ret.environment;
    PARSE_OPTIONAL_KEY("bodies", ret.bodies);
    PARSE_OPTIONAL_KEY("setpoints", ret.setpoints);
    PARSE_OPTIONAL_KEY("controllers", ret.controllers);
    PARSE_OPTIONAL_KEY("commands", ret.commands);
    return ret;
}
