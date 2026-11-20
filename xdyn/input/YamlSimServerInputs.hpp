#ifndef EXTERNAL_DATA_STRUCTURES_INC_YAMLSIMSERVERINPUTS_HPP_
#define EXTERNAL_DATA_STRUCTURES_INC_YAMLSIMSERVERINPUTS_HPP_
#include <map>
#include <string>
#include <vector>

#include "xdyn/input/YamlController.hpp"
#include "xdyn/input/YamlState.hpp"

struct YamlSimServerInputs {
    double Dt{};
    std::vector<YamlState> states;
    std::map<std::string, double> commands;
    std::vector<std::string> requested_output;
    std::vector<YamlController> controllers;
};

#endif /* EXTERNAL_DATA_STRUCTURES_INC_YAMLSIMSERVERINPUTS_HPP_ */
