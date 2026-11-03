/*
 * Simpson.cpp
 *
 * \date 23 avr. 2013, 15:39:50
 * \author cec
 */

#include "Simpson.hpp"
#include <cmath>

class ssc::integrate::Simpson::Impl
{
    public:
        Impl()
        {

        }
        double adaptive_simpson(const Function& f, const double a, const double b, const double epsilon,
                const double S, const double fa, const double fb, const double fc, const int bottom) const
        {
          const double c = (a + b)/2.;
          const double h = b - a;
          const double d = (a + c)/2.;
          const double e = (c + b)/2.;
          const double fd = f(d);
          const double fe = f(e);
          const double Sleft = (h/12)*(fa + 4*fd + fc);
          const double Sright = (h/12)*(fc + 4*fe + fb);
          const double S2 = Sleft + Sright;
          if (bottom <= 0 || fabs(S2 - S) <= 15*epsilon)
            return S2 + (S2 - S)/15;
          return adaptive_simpson(f, a, c, epsilon/2., Sleft,  fa, fc, fd, bottom-1) +
                 adaptive_simpson(f, c, b, epsilon/2., Sright, fc, fb, fe, bottom-1);
        }

        double integrate(const Function& f, const double a, const double b, const double eps) const
        {
            const double c = (a + b)/2, h = b - a;
            const double fa = f(a), fb = f(b), fc = f(c);
            const double S = (h/6.)*(fa + 4*fc + fb);
            return adaptive_simpson(f, a, b, eps, S, fa, fb, fc, 10);
        }


};

ssc::integrate::Simpson::Simpson() : ssc::integrate::Integrator(), pimpl(new Impl())
{
}

ssc::integrate::Simpson::Simpson(const Function& f_) : ssc::integrate::Integrator(f_), pimpl(new Impl())
{
}

double ssc::integrate::Simpson::integrate_impl(const Function& f_, double a, double b, double eps)
{
    return pimpl->integrate(f_,a,b,eps);
}
