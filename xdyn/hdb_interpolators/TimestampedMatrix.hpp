/*
 * TimestampedMatrix.hpp
 *
 *  Created on: Nov 13, 2014
 *      Author: cady
 */

#ifndef TIMESTAMPEDMATRIX_HPP_
#define TIMESTAMPEDMATRIX_HPP_

#include <array>
#include <utility> // std::pair
#include <vector>

using line = std::array<double,6>;
using Matrix = std::array<line,6>;
using TimestampedMatrix = std::pair<double,Matrix>;
using TimestampedMatrices = std::vector<TimestampedMatrix>;

struct RAOData
{
    RAOData() : periods(), psi(), values() {}
    std::vector<double> periods;
    std::vector<double> psi;
    std::array<std::vector<std::vector<double> >, 6 > values;
};
#endif /* TIMESTAMPEDMATRIX_HPP_ */
