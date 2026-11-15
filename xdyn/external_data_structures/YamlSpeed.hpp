/*
 * YamlSpeed.hpp
 *
 *  Created on: 16 avr. 2014
 *      Author: cady
 */

#ifndef YAMLSPEED_HPP_
#define YAMLSPEED_HPP_

#include <string>

struct YamlSpeed
{
    YamlSpeed();
    std::string frame;
    double u{0};
    double v{0};
    double w{0};
    double p{0};
    double q{0};
    double r{0};
};

#endif /* YAMLSPEED_HPP_ */
