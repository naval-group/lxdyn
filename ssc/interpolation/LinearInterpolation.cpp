/*
 * LinearInterpolation.cpp
 *
 * \date 8 nov. 2012, 14:32:11
 * \author cec
 */

#include "ssc/interpolation/LinearInterpolation.hpp"
#include <cmath>

using namespace ssc::interpolation;

LinearInterpolation::LinearInterpolation(const double& xmin_,
                                         const double& xmax_,
                                         const std::vector<double>& y_) : ConstantStepInterpolator(xmin_,xmax_,y_),
                                         x0(std::vector<double>(n-1,0)),
                                         x1(std::vector<double>(n-1,0)),
                                         y0(std::vector<double>(n-1,0)),
                                         y1(std::vector<double>(n-1,0))
{
}


void LinearInterpolation::compute_coefficients_for_ith_interval(const double val, const size_t i)
{
    (void) i;
    (void) val;
    x0[idx] = xmin + double(idx)*delta;
    x1[idx] = x0[idx] + delta;
    y0[idx] = y.at(idx);
    y1[idx] = (n==1) ? y0[idx] : y.at(idx+1);
}

double LinearInterpolation::get_f() const
{
    return y0[idx]+(val_sat-x0[idx])*(y1[idx]-y0[idx])/delta;
}

double LinearInterpolation::get_df(const size_t derivative_order) const
{
    if (derivative_order==0) return get_f();
    if (derivative_order==1) return (y1[idx]-y0[idx])/delta;
                             return 0;
}

