/*
 * TrapezoidalIntegration.cpp
 *
 * \date 23 avr. 2013, 14:47:22
 * \author cec
 */

#include "TrapezoidalIntegration.hpp"
#include <cmath>

class ssc::integrate::TrapezoidalIntegration::Impl
{
    public:
        Impl()
        {

        }

        double integrate(const Function& f, const double a, const double b, const double eps) const
        {
            return integrate_n_steps(f, a, b, (size_t)floor(1./fabs(eps)));
        }

        double integrate_n_steps(const Function& f, const double a, const double b, const size_t n) const
        {
            const double h = (b - a) / (double)n;
            double S = (f(a) + f(b))/2.;
            for (size_t i  = 1 ; i < n ; ++i)
            {
                S += f(a + (double)i * h);
            }
            return S* h;
        }

};

ssc::integrate::TrapezoidalIntegration::TrapezoidalIntegration() : Integrator(), pimpl(new Impl())
{
}

ssc::integrate::TrapezoidalIntegration::TrapezoidalIntegration(const Function& f_) : Integrator(f_), pimpl(new Impl())
{
}

double ssc::integrate::TrapezoidalIntegration::integrate_n_steps(double a, double b, size_t n) const
{
    return pimpl->integrate(f,a,b,1./(double)n);
}

double ssc::integrate::TrapezoidalIntegration::integrate_impl(const Function& f_, double a, double b, double eps)
{
    return pimpl->integrate(f_,a,b,eps);
}
