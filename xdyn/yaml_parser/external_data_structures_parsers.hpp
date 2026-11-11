/*
 * external_data_structures_parsers.hpp
 *
 *  Created on: 15 avr. 2014
 *      Author: cady
 */

#ifndef EXTERNAL_DATA_STRUCTURES_PARSERS_HPP_
#define EXTERNAL_DATA_STRUCTURES_PARSERS_HPP_

#include "xdyn/external_data_structures/YamlSimulatorInput.hpp"
// The templates below extract with node >> value. That operator is a global-namespace
// template, so ADL cannot find it at instantiation — it has to be visible here, where
// the templates are defined.
#include "xdyn/yaml_parser/yaml_compat.h"

#include <optional>
#include <yaml-cpp/yaml.h>

size_t try_to_parse_positive_integer(const YAML::Node& node, const std::string& key);
void operator >> (const YAML::Node& node, YamlAngle& a);
void operator >> (const YAML::Node& node, YamlBlockedDOF& b);
void operator >> (const YAML::Node& node, YamlBody& b);
void operator >> (const YAML::Node& node, YamlCoordinates& c);
void operator >> (const YAML::Node& node, YamlDynamics& d);
void operator >> (const YAML::Node& node, YamlEnvironmentalConstants& f);
void operator >> (const YAML::Node& node, YamlFilteredStates& p);
void operator >> (const YAML::Node& node, YamlModel& m);
void operator >> (const YAML::Node& node, YamlPoint& p);
void operator >> (const YAML::Node& node, YamlPosition& m);
void operator >> (const YAML::Node& node, YamlRotation& g);
void operator >> (const YAML::Node& node, YamlSpeed& s);
void parse_point_with_name(const YAML::Node& node, YamlPoint& p, const std::string& name);
void parse_YamlDynamics6x6Matrix(const YAML::Node& node, YamlDynamics6x6Matrix& m, const bool parse_frame, const std::string& frame_name="");
YamlBlockedDOF parse(const std::string& yaml);

std::vector<double> extract_vector_of_doubles(const YAML::Node& node, const std::string& key);

template <typename T>
void try_to_parse(const YAML::Node& node, const std::string& key, T& value)
{
    if (node[key]) node[key] >> value;
}

template <typename T>
void parse_optional(const YAML::Node& node, const std::string& key, std::optional<T>& opt)
{
    if (node[key])
    {
        T value;
        node[key] >> value;
        opt = value;
    }
}

#endif /* EXTERNAL_DATA_STRUCTURES_PARSERS_HPP_ */
