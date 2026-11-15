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
    double g{0};
    double rho{0};
    std::optional<double> rho_air;
    double nu{0};
};

#endif /* YAMLENVIRONMENTALCONSTANTS_HPP_ */
