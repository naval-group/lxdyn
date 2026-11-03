/*
 * Burcher.cpp
 *
 * \date Dec 11, 2014
 * \author cec
 */

#include "ssc/integrate/Burcher.hpp"
#include "ssc/macros/test_macros.hpp"
#include <boost/math/special_functions/sinc.hpp>
#include <cmath>

ssc::integrate::Burcher::Burcher() : Integrator() , tau(0)
{

}

ssc::integrate::Burcher::Burcher(const Function& f) : Integrator(f), tau(0)
{
}

ssc::integrate::Burcher::Burcher(const Function& f, const double tau_) : Integrator(f), tau(tau_)
{
}

double ssc::integrate::Burcher::compute_for(const double tau_, const double a, const double b, const double eps)
{
    tau = tau_;
    return Integrator::integrate(f, a, b, eps);
}

double ssc::integrate::Burcher::integrate_n_steps(const Function& f_, double a, double b, size_t n)
{
    f = f_;
    double omega_prev = a;
    double f_omega_prev = f(a);
    double res = 0;
    const double h = (b-a)/((double)n-1);
    for (size_t i = 0 ; i < n -1 ; ++i)
    {
        const double omega = a + ((double)i+1)*h;
        const double f_omega = f(omega);
        const double Omega = (omega + omega_prev)/2.;
        const double sinc1 = boost::math::sinc_pi(tau*Omega);
        const double sinc2 = boost::math::sinc_pi(tau*(omega - omega_prev)/2.);
        res += (f_omega_prev - f_omega)*Omega*sinc1*sinc2;
        omega_prev = omega;
        f_omega_prev = f_omega;
    }
    return res;
}

double ssc::integrate::Burcher::integrate_impl(const Function& f_, double a, double b, double eps)
{
    return integrate_n_steps(f_,a,b,(size_t)std::floor(1./eps));
}

