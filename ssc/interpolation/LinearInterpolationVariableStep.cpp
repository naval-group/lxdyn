/*
 * LinearInterpolationVariableStep.cpp
 *
 * \date 10 avr. 2013, 12:54:12
 * \author cec
 */

#include "ssc/interpolation/LinearInterpolationVariableStep.hpp"
#include "ssc/interpolation/PiecewiseConstantVariableStep.hpp"
#include <algorithm> // std::min, std::max
#include <cmath>

using namespace ssc::interpolation;

class Linear
{
    public:
        Linear() : slope(0), intersection_with_origin(0)
        {
        }

        Linear(const double& slope_, const double& intersection_with_origin_) : slope(slope_), intersection_with_origin(intersection_with_origin_)
        {
        }

        double slope;
        double intersection_with_origin;
};

namespace ssc
{
    namespace interpolation
    {
        std::vector<Linear> compute_linear_coefficients(const std::vector<double>& x, const std::vector<double>& y);
        std::vector<Linear> compute_linear_coefficients(const std::vector<double>& x, const std::vector<double>& y)
        {
            const size_t n = x.size();
            std::vector<Linear> ret;
            PiecewiseConstantVariableStep<double>(x,y); // Checks if x is in strictly increasing order & that x & y have same size
            for (size_t i = 0 ; i < n-1 ; ++i)
            {
                const double a = (y.at(i+1) - y.at(i)) / (x.at(i+1) - x.at(i));
                const double b = y.at(i) - a*x.at(i);
                ret.push_back(Linear(a,b));
            }

            ret.push_back(ret.back());
            return ret;
        }
    }
}

class LinearInterpolationVariableStep::LinearInterpolationVariableStepImpl
{
    public:
        LinearInterpolationVariableStepImpl(const std::vector<double>& x_, const std::vector<double>& y) : coeff_computer(PiecewiseConstantVariableStep<Linear>(x_, compute_linear_coefficients(x_,y),true)), coeffs(std::vector<Linear>(x_.size()-1,Linear())), x0_(0), x(x_)
        {

        }

        void set_computed_value(const double x0, const size_t i)
        {
            coeffs[i] = coeff_computer.typed_f(std::min(x.back(),std::max(x.front(),x0)));
        }

        double get_val(const size_t i) const
        {
            if (x0_ < x.front()) return coeffs.front().slope*x.front() + coeffs.front().intersection_with_origin;
            if (x0_ > x.back())  return coeffs.back().slope*x.back() + coeffs.back().intersection_with_origin;
                                 return coeffs[i].slope*x0_ + coeffs[i].intersection_with_origin;
        }

        double get_derivative(const size_t i) const
        {
            if ((x0_ < x.front()) or (x0_ > x.back())) return 0;
            return coeffs[i].slope;
        }

    private:
        LinearInterpolationVariableStepImpl();
        PiecewiseConstantVariableStep<Linear> coeff_computer;
        std::vector<Linear> coeffs;

    public:
        double x0_;
        std::vector<double> x;
};

size_t find_index_of_interval(const double x, const std::vector<double>& xs);
size_t find_index_of_interval(const double x, const std::vector<double>& xs)
{
    size_t idx2 = xs.size();
    if (idx2==0) return 0;
    if (x<=xs.front()) return 0;
    if (x>=xs.back()) return idx2-2;

    size_t idx1 = 0;

    while (idx2-idx1>1)
    {
        const size_t idx_med = (idx1+idx2)/2;
        if (x >= xs[idx_med]) idx1 = idx_med;
        else                  idx2 = idx_med;
    }
    return idx1;
}

void LinearInterpolationVariableStep::find_index_of_interval_containing(const double val)
{
    pimpl->x0_ = val;
    idx = find_index_of_interval(val, pimpl->x);
}


LinearInterpolationVariableStep::LinearInterpolationVariableStep(const std::vector<double>& x, const std::vector<double>& y) :
        ConstantStepInterpolator(0,1,y),pimpl(new LinearInterpolationVariableStepImpl(x,y))
{
    xmin = x.front();
    xmax = x.back();
}

void LinearInterpolationVariableStep::compute_coefficients_for_ith_interval(const double x0, const size_t i)
{
    pimpl->set_computed_value(x0,i);
}

double LinearInterpolationVariableStep::get_f() const
{
    return pimpl->get_val(idx);
}

double LinearInterpolationVariableStep::get_df(const size_t derivative_order) const
{
    if (derivative_order==0) return get_f();
    if (derivative_order==1) return pimpl->get_derivative(idx);
                             return 0;
}
