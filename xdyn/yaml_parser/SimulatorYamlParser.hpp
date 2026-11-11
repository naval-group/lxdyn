/*
 * SimulatorYamlParser.hpp
 *
 *  Created on: 15 avr. 2014
 *      Author: cady
 */

#ifndef SIMULATORYAMLPARSER_HPP_
#define SIMULATORYAMLPARSER_HPP_

#include "xdyn/external_data_structures/YamlSimulatorInput.hpp"
#include <string>

class SimulatorYamlParser
{
    public:
        explicit SimulatorYamlParser(const std::string& data);
        YamlSimulatorInput parse() const;

    private:
        const std::string contents;
};

#endif /* SIMULATORYAMLPARSER_HPP_ */
