/*
 * CubicRunoutSplines.cpp
 *
 * \date 1 août 2012, 09:57:37
 * \author cec
 */

#include "ssc/interpolation/CubicRunoutSplines.hpp"

using namespace ssc::interpolation;

CubicRunoutSplines::CubicRunoutSplines() : Splines()
{
    M = (h != 0) ? compute_second_derivative() : std::vector<double>(2,0);
}

CubicRunoutSplines::CubicRunoutSplines(const double& xmin_, const double& xmax_, const std::vector<double>& y) : Splines(xmin_, xmax_,y)
{
    if (n>1)
    {
        M = (h != 0) ? compute_second_derivative() : std::vector<double>(2,0);
    }
    else
    {
        M = std::vector<double>(1,0.);
    }
}

double CubicRunoutSplines::get_endpoint_value() const
{
    return 6;
}
