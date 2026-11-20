/*
 * SimulatorYamlParser.hpp
 *
 *  Created on: 15 avr. 2014
 *      Author: cady
 */

#ifndef SIMULATORYAMLPARSER_HPP_
#define SIMULATORYAMLPARSER_HPP_

#include <string>

#include "xdyn/input/YamlSimulatorInput.hpp"

class SimulatorYamlParser {
public:
    explicit SimulatorYamlParser(const std::string& data);
    YamlSimulatorInput parse() const;

private:
    const std::string contents;
};

#endif /* SIMULATORYAMLPARSER_HPP_ */
