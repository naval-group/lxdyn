/*
 * Filon.cpp
 *
 * \date Dec 10, 2014
 * \author cec
 */

#include "ssc/integrate/Filon.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

ssc::integrate::Filon::Filon() : QuadPack(), tau(0)
{

}

ssc::integrate::Filon::Filon(const Function& f_) : QuadPack(f_), tau(0)
{
}

ssc::integrate::Filon::Filon(const Function& f_, const double tau_) : QuadPack(f_), tau(tau_)
{
}

double ssc::integrate::Filon::compute_for(const double tau_, const double a, const double b, const double eps)
{
    tau = tau_;
    return integrate_impl(f, a, b, eps);
}

extern "C" int filon(double f(void*,double*), void *obj, double *a, double *b, double *
        fpa, double *fpb, double *fba, double *fbb, double *w,
        double *eps, int *maxh, int *lc, int *ls, double
        *c__, double *s);

double ssc::integrate::Filon::integrate_impl(const Function& f_, double a, double b, double eps)
{
    f = f_;
    int maxh = 100;
    double res_cos = 0;
    double res_sin = 0;
    double fpa = 0;
    double fpb = 0;
    double fba = 0;
    double fbb = 0;
    int lc = 1;
    int ls = 1-lc;
    filon(integrand<Filon>,
           (void*)this,
           &a,
           &b,
           &fpa,
           &fpb,
           &fba,
           &fbb,
           &tau,
           &eps,
           &maxh,
           &lc,
           &ls,
           &res_cos,
           &res_sin);
    return res_cos;
}
