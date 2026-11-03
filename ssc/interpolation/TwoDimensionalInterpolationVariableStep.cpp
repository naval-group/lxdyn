/*
 * TwoDimensionalInterpolationVariableStep.cpp
 *
 * \date Dec 17, 2014
 * \author cec
 */
#include "TwoDimensionalInterpolationVariableStep.hpp"

ssc::interpolation::TwoDimensionalInterpolationVariableStep::TwoDimensionalInterpolationVariableStep() : y_interpolators_for_each_x(),
        x()
{
}

ssc::interpolation::TwoDimensionalInterpolationVariableStep::TwoDimensionalInterpolationVariableStep(const std::vector<double>& x_, const std::vector<double>& y, const std::vector<std::vector<double> >& M) :
        y_interpolators_for_each_x(),
        x(x_)
{
    for (auto it = M.begin() ; it != M.end() ; ++it)
    {
        y_interpolators_for_each_x.push_back(LinearInterpolationVariableStep(y, *it));
    }
}

double ssc::interpolation::TwoDimensionalInterpolationVariableStep::f(const double x0, const double y0)
{
    std::vector<double> interpolated_values_for_x_fixed;
    for (auto it = y_interpolators_for_each_x.begin() ; it != y_interpolators_for_each_x.end() ; ++it)
    {
        interpolated_values_for_x_fixed.push_back(it->f(y0));
    }
    LinearInterpolationVariableStep final_interpolation(x,interpolated_values_for_x_fixed);
    const double ret = final_interpolation.f(x0);
    return ret;
}
