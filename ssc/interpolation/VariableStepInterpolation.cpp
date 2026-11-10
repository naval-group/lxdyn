/*
 * VariableStepInterpolation.cpp
 *
 * \date 10 sept. 2013, 11:55:04
 * \author cec
 */

#include "ssc/interpolation/VariableStepInterpolation.hpp"

#include <memory>
#include "ssc/interpolation/IndexFinder.hpp"

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#define not !
#endif

using namespace ssc::interpolation;

VariableStepInterpolation::VariableStepInterpolation() : index(std::make_shared<IndexFinder>(std::vector<double>(),false)), idx(0), x0(0)
{

}

VariableStepInterpolation::~VariableStepInterpolation()
{
}

VariableStepInterpolation::VariableStepInterpolation(const std::vector<double>& x, const bool allow_queries_outside_bounds) : index(std::make_shared<IndexFinder>(x,not(allow_queries_outside_bounds))), idx(0), x0(0)
{
}

void VariableStepInterpolation::update_coefficients_if_necessary(const double x0_)
{
    x0 = x0_;
    idx = int(index->compute(x0));
}

