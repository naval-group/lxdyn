/*
 * IndexFinder.cpp
 *
 * \date 10 sept. 2013, 10:03:21
 * \author cec
 */

#include "ssc/interpolation/IndexFinder.hpp"
#include <algorithm> // std::min, std::max
#include <cfloat>
#include <sstream>

using namespace ssc::interpolation;

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

void IndexFinder::check_x_is_strictly_increasing(const std::vector<double>& x)
{
    for (size_t i = 1; i < (size_t)n; ++i)
    {
        if (x[i] <= x[i - 1])
        {
            std::stringstream ss;
            ss << "x should be in strictly increasing order: x[" << i - 1
                    << "] = " << x.at(i - 1) << ", but x" << "[" << i << "] = "
                    << x.at(i);
            THROW(__PRETTY_FUNCTION__, IndexFinderException, ss.str());
        }
    }
}

IndexFinder::IndexFinder(const std::vector<double>& x, const bool throw_if_outside_bounds_) : n(int(x.size())),
                                                                                             x_(x),
                                                                                             xmin(0),
                                                                                             xmax(0),
                                                                                             idx_low(0),
                                                                                             throw_if_outside_bounds(throw_if_outside_bounds_)
{
    if (!(x.empty()))
    {
        xmin = x.front();
        xmax = x.back();
    }
    check_x_is_strictly_increasing(x);
}

size_t IndexFinder::compute(const double x0)
{
    const bool outside_bounds = (x0 < xmin-DBL_EPSILON) || (x0 > xmax+DBL_EPSILON);
    if (outside_bounds && throw_if_outside_bounds)
    {
        std::stringstream ss;
        ss << "Received x0 = " << x0 << ", but x0 should be within [" << xmin << "," << xmax << "] (xmin-x0 = " << xmin-x0 << " and x0-xmax = " << x0-xmax << ")";
        THROW(__PRETTY_FUNCTION__, IndexFinderException, ss.str());
    }
    if (x0 < xmin)       return 0;
    if (x0 > xmax)       return (size_t)n-1;
    int ilo = idx_low;
    int ihi = ilo + 1;
    if (ihi >= n-1)
    {
        if (x0 >= xmax)  return greater_than_max(ilo);
        if (n <= 1)      return lower_than_min(ilo);
        ilo = n - 2;
        ihi = n - 1;
    }
    if (x0 >= x_[(size_t)ihi])   return increase_ihi_to_capture_x(ilo, ihi, x0);
    if (x0 >= x_[(size_t)ilo])   return between_min_and_max(ilo);
                                 return decrease_ilo_to_capture_x(ilo, ihi, x0);
}

void IndexFinder::adjust_left(int& left) const
{
    while (true)
    {
        if (left == 0) break;
        left = left - 1;
        if (x_[(size_t)left] < xmax) break;
    }
}

size_t IndexFinder::greater_than_max(const int ilo)
{
      idx_low = ilo;
      return (size_t)n-1;
}

size_t IndexFinder::lower_than_min(const int ilo)
{
      idx_low = ilo;
      return 0;
}

size_t IndexFinder::between_min_and_max(const int ilo)
{
      idx_low = ilo;
      return (size_t)ilo;
}

size_t IndexFinder::bisection (const int ilo, const int ihi, const double x)
{
    const int middle = (ilo + ihi)/2;
    if (middle == ilo)           return between_min_and_max(ilo);
    if (x < x_[(size_t)middle])  return bisection(ilo, middle, x);
                                 return bisection(middle, ihi, x);
}

size_t IndexFinder::increase_ihi_to_capture_x(int& ilo, int& ihi, const double x)
{
    int istep = 1;
    while(true)
    {
        ilo = ihi;
        ihi = ilo + istep;
        if (ihi >= n-1)              break;
        if (x < x_[(size_t)ihi])     return bisection(ilo, ihi, x);
        istep = istep*2;
    }
    if (x >= xmax)
    {
        int left = int(greater_than_max(ilo));
        adjust_left(left);
        return (size_t)left;
    }
    ihi = n-1;
    ilo = std::max(0,std::min(ilo, n-1));
    return bisection(ilo, ihi, x);
}

size_t IndexFinder::decrease_ilo_to_capture_x(int& ilo, int& ihi, const double x)
{
    int istep = 1;
    while (ilo > 0)
    {
        ihi = ilo;
        ilo = std::max(0,ihi - istep);
        if (x >= x_[(size_t)ilo]) return bisection(ilo, ihi, x);
        istep = istep*2;
    }
    ilo = 0;
    if (x < xmin)                 return lower_than_min(ilo);
    return bisection(ilo, ihi, x);
}
