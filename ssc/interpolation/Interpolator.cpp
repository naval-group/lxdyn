/*
 * Interpolator.cpp
 *
 * \date 9 sept. 2013, 11:11:02
 * \author cec
 */

#include "ssc/interpolation/Interpolator.hpp"

using namespace ssc::interpolation;

Interpolator::Interpolator()
{
}

Interpolator::~Interpolator()
{

}

double Interpolator::f(const double x)
{
    update_coefficients_if_necessary(x);
    return get_f();
}

double Interpolator::df(const double x, const size_t derivative_order)
{
    update_coefficients_if_necessary(x);
    return get_df(derivative_order);
}
