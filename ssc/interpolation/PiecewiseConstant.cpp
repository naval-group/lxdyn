/*
 * PiecewiseConstant.cpp
 *
 * \date 18 févr. 2013, 13:07:18
 * \author cec
 */

#include "ssc/interpolation/PiecewiseConstant.hpp"
#include "ssc/interpolation/PiecewiseConstantException.hpp"
#include <algorithm> // std::min, std::max
#include <cmath>

using namespace ssc::interpolation;

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

PiecewiseConstant::PiecewiseConstant(const double& xmin_, const double& xmax_, const std::vector<double>& y_) :
ConstantStepInterpolator(xmin_,xmax_,y_)
{
    if (n < 2)
    {
        THROW(__PRETTY_FUNCTION__, PiecewiseConstantException, "Needs at least two points in vector y");
    }
}

void PiecewiseConstant::update_coefficients_if_necessary(const double val)
{
    idx = (size_t)std::max(0,(int)std::min(floor((val-xmin)/(xmax-xmin)*double(n)),(double)n-1));
}

void PiecewiseConstant::compute_coefficients_for_ith_interval(const double x0, const size_t i)
{
    (void) i;
    (void) x0;
}

double PiecewiseConstant::get_f() const
{
    return y.at(idx);
}

double PiecewiseConstant::get_df(const size_t derivative_order) const
{
    if (derivative_order==0) return get_f();
                             return 0;
}
