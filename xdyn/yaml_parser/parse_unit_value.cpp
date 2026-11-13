#include "parse_unit_value.hpp"

#include <ssc/decode_unit/DecodeUnit.h>

void xdyn::yaml_parser::parse_uv(const YAML::Node& node, std::vector<double>& d)
{
    std::string unit = node["unit"].as<std::string>();
    const double factor = ssc::decode_unit::decodeUnit(unit);
    d = node["values"].as<std::vector<double>>();
    for (std::vector<double>::iterator it = d.begin(); it != d.end(); ++it) {
        *it *= factor;
    }
}

void xdyn::yaml_parser::parse_uv(const YAML::Node& node, double& d)
{
    std::string unit = node["unit"].as<std::string>();
    d = node["value"].as<double>() * ssc::decode_unit::decodeUnit(unit);
}
