/*
 * YamlEnvironmentalConstants.hpp
 *
 *  Created on: Jun 19, 2014
 *      Author: cady
 */

#ifndef YAMLENVIRONMENTALCONSTANTS_HPP_
#define YAMLENVIRONMENTALCONSTANTS_HPP_

#include <optional>

struct YamlEnvironmentalConstants
{
    YamlEnvironmentalConstants();
    double g;
    double rho;
    std::optional<double> rho_air;
    double nu;
};

#endif /* YAMLENVIRONMENTALCONSTANTS_HPP_ */
