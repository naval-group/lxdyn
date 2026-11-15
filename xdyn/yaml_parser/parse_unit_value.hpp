#ifndef XDYN_PARSE_UNIT_VALUE_HPP_
#define XDYN_PARSE_UNIT_VALUE_HPP_

#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>

namespace xdyn
{
    namespace yaml_parser
    {
        struct UV
        {
            UV() :  unit("") {}
            double value{0};
            std::string unit;
        };

        void parse_uv(const YAML::Node& node, double& d);
        void parse_uv(const YAML::Node& node, std::vector<double>& d);
    }
}

#endif /* XDYN_PARSE_UNIT_VALUE_HPP_ */
