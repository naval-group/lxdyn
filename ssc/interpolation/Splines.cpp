/*
 * Splines.cpp
 *
 * \date 8 juin 2012, 17:14:15
 * \author cec
 */

typedef long int integer;
typedef double doublereal;

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

extern "C"
{
    int dgtsv_(integer *n, integer *nrhs, doublereal *dl,
        doublereal *d__, doublereal *du, doublereal *b, integer *ldb, integer
        *info);
}

#include "ssc/interpolation/Splines.hpp"
#include "ssc/interpolation/SplinesException.hpp"
#include "ssc/interpolation/VectorOfEquallySpacedNumbers.hpp"

#include <cmath>
#include <cstdio>

using namespace ssc::interpolation;

Splines::Splines() :
        M(std::vector<double>()),
        h(0),
        n(0),
        x_xi(0),
        a(std::vector<double>()),
        b(std::vector<double>()),
        c(std::vector<double>()),
        d(std::vector<double>())
{
}

Splines::Splines(const double xmin_, const double xmax_, const std::vector<double>& y) :
        ConstantStepInterpolator(xmin_,xmax_,y),
        M(std::vector<double>()),
        h(VectorOfEquallySpacedNumbers(xmin_,xmax_,y.size()).get_delta()),
        n(y.size()),
        x_xi(0),
        a(std::vector<double>()),
        b(std::vector<double>()),
        c(std::vector<double>()),
        d(std::vector<double>())
{
    if (n==0)
    {
        THROW(__PRETTY_FUNCTION__, SplinesException, "There needs to be at least one point to define the spline (y vector is empty)");
    }
    a = std::vector<double>(n-1,0);
    b = std::vector<double>(n-1,0);
    c = std::vector<double>(n-1,0);
    d = std::vector<double>(n-1,0);
}

void Splines::compute_coefficients_for_ith_interval(const double x0, const size_t i)
{
    if (n>1)
    {
        auto coeff = compute_cubic_coeff_for_x0(x0);
        a[i] = coeff.a;
        b[i] = coeff.b;
        c[i] = coeff.c;
        d[i] = coeff.d;
    }
}

CubicCoefficients Splines::compute_cubic_coeff_for_x0(const double& x0)
{
    find_index_of_interval_containing(x0);
    return get_cubic_coefficients(M[idx], M[idx+1], y[idx],y[idx+1]);
}

void Splines::find_index_of_interval_containing(const double val)
{
    ConstantStepInterpolator::find_index_of_interval_containing(val);
    const double xi = xmin + (xmax-xmin)*double(idx)/double(n-1);
    x_xi = val-xi;
}

double Splines::get_f() const
{
    if (n==0) return 0;
    return (n==1) ? y.back() : d[idx]+x_xi*(c[idx]+x_xi*(b[idx]+x_xi*a[idx]));
}

double Splines::get_df(const size_t derivative_order) const
{
    if (n==0) return 0;
    if (derivative_order==0) return get_f();
    if (derivative_order==1) return (n==1) ? 0 : c[idx]+x_xi*(2*b[idx]+x_xi*3*a[idx]);
    if (derivative_order==2) return (n==1) ? 0 : 2*b[idx]+x_xi*6*a[idx];
                             return 0;
}


std::vector<ParabolicCoefficients> Splines::get_parabolic_coefficients()
{
    std::vector<ParabolicCoefficients> ret;
    for (size_t i = 0 ; i < n-1 ; ++i)
    {
        auto coeff = compute_cubic_coeff_for_x0(xmin+(double(i)+0.5)*h);
        ret.push_back(ParabolicCoefficients(3*coeff.a,2*coeff.b,coeff.c));
    }
    return ret;
}

std::pair<double,double> Splines::find_position_and_value_of_minimum(const size_t i)//const ParabolicCoefficients& c, const double x0, const double x1)
{
    const double l = xmin + double(i)*h;
    const double u = l + h;
    compute_coefficients_for_ith_interval(xmin + (double(i)+0.5)*h,i);

    const double delta = b[i]*b[i]-3*a[i]*c[i];
    double xmin = u+h; // outside bounds by default
    double ymin = 123456;
    if (delta>0)
    {
        const double sqrt_delta = sqrt(delta);
        const double x1 = l+(-b[i]-sqrt_delta)/(3.*a[i]);
        const double x2 = l+(-b[i]+sqrt_delta)/(3.*a[i]);
        double y1 = 0;
        double y2 = 0;
        if ((x1 >= l) && (x1 <= u))
        {
            y1 = f(x1);
            if ((x2 >= l) && (x2 <= u))
            {
                y2 = f(x2);
                if (y1 < y2)
                {
                    xmin = x1;
                    ymin = y1;
                }
                else
                {
                    xmin = x2;
                    ymin = y2;
                }
            }
            else
            {
                xmin = x1;
                ymin = y1;
            }
        }
        else
        {
            if ((x2 >= l) && (x2 <= u))
            {
                y2 = f(x2);
                xmin = x2;
                ymin = y2;
            }
        }
    }
    if (delta==0)
    {
        xmin = l+(-b[i])/(3.*a[i]);
        ymin = f(xmin);
    }

    const bool xmin_outside_bounds = (xmin < l) || (xmin > u);
    if (xmin_outside_bounds)
    {
        const double yl = f(l);
        const double yu = f(u);
        return (yl<yu) ? std::make_pair(l,yl) : std::make_pair(u,yu);
    }
    return std::make_pair(xmin,ymin);
}

std::pair<double,double> Splines::find_position_and_value_of_minimum()
{
    if (n==0) return std::pair<double,double>();
    std::pair<double,double> ret = find_position_and_value_of_minimum(0);
    for (size_t i = 1 ; i < (n-1) ; ++i)
    {
        const auto pos_min = find_position_and_value_of_minimum(i);
        if (pos_min.second < ret.second) ret = pos_min;
    }

    return ret;
}

std::vector<double> Splines::compute_second_derivative() const
{
    if (n==2) return std::vector<double>(2,0);
    if (n==3)
    {
        std::vector<double> ret(3,0);
        ret[1] = 3./2./h/h*(y[0]-2.*y[1]+y[2]);
        return ret;
    }
    integer *n_ = new integer;
    integer *nrhs = new integer;
    *n_ = (integer)n-2;
    *nrhs = 1;
    doublereal *dl = new doublereal[*n_];
    doublereal *d__ = new doublereal[*n_+2];
    doublereal *du = new doublereal[*n_];
    doublereal *b = new doublereal[*n_+2];
    integer *ldb = new integer;
    integer *info = new integer;
    *ldb = *n_;
    d__[0] = get_endpoint_value();
    d__[*n_+1] = get_endpoint_value();
    b[0] = 0;
    for (size_t i = 1 ; i <= (size_t)*n_ ; ++i)
    {
        dl[i-1] = 1;
        du[i-1] = 1;
        d__[i] = 4;
        b[i] = 6./h/h*(y[i-1]-2.*y[i]+y[i+1]);
    }
    b[*n_+1] = 0;
    dgtsv_(n_, nrhs, dl, d__, du, b+1, ldb, info);
    const std::vector<double> ret(b,b+*n_+2);
    delete n_;
    delete nrhs;
    delete [] dl;
    delete [] d__;
    delete [] du;
    delete [] b;
    delete info;
    delete ldb;
    return ret;
}

CubicCoefficients Splines::get_cubic_coefficients(const double& M1, const double& M2, const double& y1, const double& y2) const
{
    const double a_ = (h!=0) ?(M2-M1)/(6.*h) : 0;
    const double b_ = M1/2.;
    const double c_ = (h!=0) ? (y2-y1)/h - (M2+2.*M1)*h/6. : 0;
    const double d_ = y1;
    return CubicCoefficients(a_,b_,c_,d_);
}

