/*
 * Rectangle.cpp
 *
 * \date Nov 28, 2014
 * \author cec
 */

#include "Rectangle.hpp"
#include <cmath>
#include <cstdlib>

ssc::integrate::Rectangle::Rectangle() : ssc::integrate::Integrator()
{
}

ssc::integrate::Rectangle::Rectangle(const Function& f) : ssc::integrate::Integrator(f)
{
}

double ssc::integrate::Rectangle::integrate_impl(const Function& f_, double a, double b, double eps)
{
    const size_t N = (size_t)std::floor(1./eps + 0.5);
    const double h = (b-a)/((double)N);
    double I = 0;
    for (size_t i = 0 ; i < N ; ++i) I += f_(a+(double)i*h);
    return h*I;
}
