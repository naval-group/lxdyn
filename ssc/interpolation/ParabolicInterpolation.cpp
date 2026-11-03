/*
 * ParabolicInterpolation.cpp
 *
 * \date 19 févr. 2013, 09:56:40
 * \author cec
 */


#include "ssc/interpolation/ParabolicInterpolation.hpp"
#include "ssc/interpolation/ParabolicInterpolationException.hpp"

#include <cmath>
#define min(a,b) (a)>(b)?b:a
#define max(a,b) (a)>(b)?a:b

using namespace ssc::interpolation;

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

ParabolicInterpolation::ParabolicInterpolation(const double& xmin_,
        const double& xmax_,
        const std::vector<ParabolicCoefficients>& coeffs) : ConstantStepInterpolator(xmin_,xmax_,std::vector<double>(coeffs.size()+1,0)), coeffs_(coeffs)
{
    n = coeffs.size()+1;
    if (coeffs.size() < 2)
    {
        THROW(__PRETTY_FUNCTION__, ParabolicInterpolationException, "Too few elements in coeffs (requires at least 2)");
    }
    delta = (xmax-xmin)/double(n-1);
}

void ParabolicInterpolation::compute_coefficients_for_ith_interval(const double val, const size_t i)
{
    (void) i;
    (void) val;

}

double ParabolicInterpolation::get_f() const
{
    double x_xi = val_sat - (xmin + delta*double(idx));
    return coeffs_.at(idx).a*x_xi*x_xi+coeffs_.at(idx).b*x_xi+coeffs_.at(idx).c;
}

double ParabolicInterpolation::get_df(const size_t derivative_order) const
{
    double x_xi = val_sat - (xmin + delta*double(idx));
    if (derivative_order==0) return get_f();
    if (derivative_order==1) return 2*coeffs_.at(idx).a*x_xi+coeffs_.at(idx).b;
    if (derivative_order==2) return 2*coeffs_.at(idx).a;
                             return 0;
}
