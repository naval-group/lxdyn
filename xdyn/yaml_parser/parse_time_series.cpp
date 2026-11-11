/*
 * parse_time_series.cpp
 *
 *  Created on: Oct 22, 2014
 *      Author: cady
 */

#include "parse_time_series.hpp"
#include "yaml_compat.h"
#include "xdyn/exceptions/InvalidInputException.hpp"

#include "xdyn/yaml_parser/parse_unit_value.hpp"

void operator >> (const YAML::Node& node, YamlTimeSeries& c)
{
    c.name = "";
    if (node["name"])
    {
        node["name"] >> c.name;
    }

    node["t"] >> c.t;
    for(auto it=node.begin();it!=node.end();++it)
    {
        std::string key = it->first.as<std::string>();
        if ((key != "name") and (key != "t"))
        {
            try
            {
                std::vector<double> values;
                xdyn::yaml_parser::parse_uv(node[key], values);
                c.values[key] = values;
            }
            catch(const YAML::Exception& e)
            {
                THROW(__PRETTY_FUNCTION__, InvalidInputException, "Something is wrong with the YAML, more specifically in the 'commands' or 'setpoints' section. When parsing the '" << key << "' values: " << e.msg);
            }
        }
    }
}

std::vector<YamlTimeSeries> parse_command_yaml(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    std::vector<YamlTimeSeries> ret;
    node["commands"] >> ret;
    return ret;
}

std::vector<YamlTimeSeries> parse_setpoint_yaml(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    std::vector<YamlTimeSeries> ret;
    node["setpoints"] >> ret;
    return ret;
}
