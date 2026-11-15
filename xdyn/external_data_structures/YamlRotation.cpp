/*
 * YamlRotation.cpp
 *
 *  Created on: 16 avr. 2014
 *      Author: cady
 */

#include "YamlRotation.hpp"

YamlRotation::YamlRotation() : order_by("")
{
}

YamlRotation::YamlRotation(
        const std::string& order_by_,
        const std::vector<std::string>& convention_):
                 order_by(order_by_), convention(convention_)
{
}
