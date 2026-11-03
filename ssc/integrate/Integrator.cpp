/*
 * Integrator.cpp
 *
 * \date 5 juin 2013, 16:07:13
 * \author cec
 */

#include "Integrator.hpp"
#include "IntegratorException.hpp"

ssc::integrate::Integrator::Integrator() : f(), function_defined(false)
{
}

ssc::integrate::Integrator::Integrator(const Function& f_) : f(f_), function_defined(true)
{
}

ssc::integrate::Integrator::~Integrator()
{
}

double ssc::integrate::Integrator::integrate_f(double a, double b, double eps)
{
    if (not(function_defined))
    {
        THROW(__PRETTY_FUNCTION__, IntegratorException, "Function is not defined.");
    }
    return integrate_impl(f, a, b, eps);
}

double ssc::integrate::Integrator::integrate(const Function& f_, double a, double b, double eps)
{
    f = f_;
    function_defined = true;
    return integrate_impl(f, a, b, eps);
}
