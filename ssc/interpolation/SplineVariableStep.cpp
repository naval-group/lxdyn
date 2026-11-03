/*
 * SplineVariableStep.cpp
 *
 * \date 9 sept. 2013, 10:47:42
 * \author cec
 */

#include "ssc/interpolation/PiecewiseConstantVariableStep.hpp"
#include "ssc/interpolation/SplineVariableStep.hpp"
#include "ssc/interpolation/SplinesException.hpp"
#include <sstream>
#include <list>
#include <algorithm> //std::sort

using namespace ssc::interpolation;

typedef long int integer;
typedef float real;
typedef double doublereal;

extern "C"
{
    int cubspl_(doublereal *tau, doublereal *c, integer *n, integer *ibcbeg, integer *ibcend);
}

class VectorsAreTheRightSize
{
    public:
        VectorsAreTheRightSize()
        {

        }

        VectorsAreTheRightSize(const std::vector<double>& x, const std::vector<double>& y)
        {
            if (x.empty())
            {
                THROW(__PRETTY_FUNCTION__, SplineVariableStepException, "x is empty");
            }
            if (x.size() != y.size())
            {
                std::stringstream ss;
                ss << "x has size " << x.size() << " and y has size " << y.size();
                THROW(__PRETTY_FUNCTION__, SplineVariableStepException, ss.str());
            }
        }
};

struct Coeff
{
    double a;
    double b;
    double c;
    double d;
};

std::vector<Coeff> get_coeffs(const std::vector<double>& x, const std::vector<double>& y);
std::vector<Coeff> get_coeffs(const std::vector<double>& x, const std::vector<double>& y)
{
    (void) y;
    std::vector<Coeff> ret;
    integer *n = new integer(integer(x.size()));
    doublereal *tau = new doublereal[*n];
    doublereal *c = new doublereal[4*(*n)];
    integer *ibcbeg = new integer(0);
    integer *ibcend = new integer(0);
    for (int i = 0 ; i < *n ; ++i)
    {
        tau[i] = x[(size_t)i];
        c[4*i + 0] = y[(size_t)i];
        c[4*i + 1] = 0;
        c[4*i + 2] = 0;
        c[4*i + 3] = 0;
    }
    cubspl_(tau, c, n, ibcbeg, ibcend);
    for (int i = 0 ; i < *n-1 ; ++i)
    {
        Coeff coeff;
        coeff.a = c[4*i + 0];
        coeff.b = c[4*i + 1];
        coeff.c = c[4*i + 2];
        coeff.d = c[4*i + 3];
        ret.push_back(coeff);
    }
    delete[] tau;
    delete[] c;
    delete n;
    delete ibcbeg;
    delete ibcend;
    return ret;
}

class SplineVariableStep::Impl
{
    public:
        Impl() : check_that(VectorsAreTheRightSize()),
                 n(0),
                 x(),
                 y(),
                 h(),
                 coeffs(),
                 xleft()
        {
        }

        Impl(const std::vector<double>& x_, const std::vector<double>& y_) : check_that(VectorsAreTheRightSize(x_,y_)),
                                                                             n(x_.size()),
                                                                             x(),
                                                                             y(),
                                                                             h(),
                                                                             coeffs(),
                                                                             xleft()
        {
            sort_xs(x_,y_);
            coeffs = get_coeffs(x,y);
            for (size_t i = 0 ; i < n ; ++i)
            {
                xleft.push_back(x[i]);
            }
        }

        void sort_xs(const std::vector<double>& x_, const std::vector<double>& y_)
        {
            std::vector<size_t> idx;
            for (size_t i = 0 ; i < n ; ++i) idx.push_back(i);
            std::sort(idx.begin(), idx.end(), [&x_](const size_t i, const size_t j)->bool{return x_[i] < x_[j];});
            for (size_t i = 0 ; i < n ; ++i)
            {
                x.push_back(x_[idx[i]]);
                y.push_back(y_[idx[i]]);
                if (i>0)
                {
                    h.push_back(x[i]-x[i-1]);
                    if (h.back() == 0)
                    {
                        std::stringstream ss;
                        ss << "Values of x at index " << idx[i-1] << " and " << idx[i] << " are equal (namely, to " << x[i] << ")";
                        THROW(__PRETTY_FUNCTION__, SplineVariableStepException, ss.str());
                    }
                }
            }
        }

    private:
        VectorsAreTheRightSize check_that;
        size_t n;

    public:
        std::vector<double> x;
        std::vector<double> y;
        std::vector<double> h;
        std::vector<Coeff> coeffs;
        std::vector<double> xleft;
};

std::vector<double> sort(std::vector<double> v);
std::vector<double> sort(std::vector<double> v)
{
    std::sort(v.begin(), v.end());
    return v;
}

SplineVariableStep::SplineVariableStep() : VariableStepInterpolation(std::vector<double>()), pimpl(TR1(shared_ptr)<Impl>(new Impl()))
{

}

SplineVariableStep::SplineVariableStep(const std::vector<double>& x, const std::vector<double>& y, const bool allow_queries_outside_bounds) :
        VariableStepInterpolation(sort(x), allow_queries_outside_bounds),
        pimpl(TR1(shared_ptr)<Impl>(new Impl(x,y)))
{
}

double SplineVariableStep::get_f() const
{
    const Coeff coeff = pimpl->coeffs[(size_t)idx];
    double x_x0 = x0 - pimpl->xleft[(size_t)idx];
    if ((x_x0 == 0) and ((size_t)idx == (pimpl->xleft.size()-1)))
    {
        return pimpl->y.back();
    }
    return  x_x0*( x_x0*( x_x0*coeff.d/3. + coeff.c)/2. + coeff.b) + coeff.a;
}

double SplineVariableStep::get_df(const size_t derivative_order) const
{
    const Coeff coeff = pimpl->coeffs[(size_t)idx];
    const double x_x0 = x0 - pimpl->xleft[(size_t)idx];
    switch (derivative_order)
    {
        case 0:
            return get_f();
            break;
        case 1:
            return x_x0*(x_x0*coeff.d/2+coeff.c)+coeff.b;
            break;
        case 2:
            return x_x0*coeff.d + coeff.c;
            break;
        case 3:
            return coeff.d;
            break;
        default:
            return 0;
    }
    return  0;
}
