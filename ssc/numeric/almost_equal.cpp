/*
 * almost_equal.cpp
 *
 * \date 26 nov. 2013, 11:22:57
 * \author cec
 */

#include "ssc/numeric/almost_equal.hpp"
#include "gtest/internal/gtest-internal.h"
#include <cmath>


template <> bool almost_equal(const double& t1, const double& t2, const double eps)
{
    if (eps>0) return fabs(t1-t2)<eps;
    const testing::internal::FloatingPoint<double> lhs(t1), rhs(t2);
    return lhs.AlmostEquals(rhs);
}

template <> bool almost_equal(const float& t1, const float& t2, const double eps)
{
    if (eps>0) return fabs(t1-t2)<eps;
    const testing::internal::FloatingPoint<float> lhs(t1), rhs(t2);
    return lhs.AlmostEquals(rhs);
}
template <> bool almost_equal(const int& t1, const int& t2, const double)
{
    return t1 == t2;
}

template <> bool almost_equal(const size_t& t1, const size_t& t2, const double)
{
    return t1 == t2;
}

template <> bool almost_equal(const std::string& t1, const std::string& t2, const double)
{
    return t1 == t2;
}

template <> bool almost_equal(const std::vector<double>& t1, const std::vector<double>& t2, const double eps)
{
    const size_t n = t1.size();
    if (t2.size() != n) return false;
    for (size_t i = 0 ; i < n ; ++i)
    {
        if (not(almost_equal(t1[i],t2[i],eps))) return false;
    }
    return true;
}
