/*
 * VectorOfEquallySpacedNumbers.cpp
 *
 * \date 31 juil. 2012, 09:06:33
 * \author cec
 */

#include "ssc/interpolation/VectorOfEquallySpacedNumbers.hpp"
#include "ssc/interpolation/VectorOfEquallySpacedNumbersException.hpp"
#include <sstream>

#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

using namespace ssc::interpolation;

VectorOfEquallySpacedNumbers::VectorOfEquallySpacedNumbers(const double& min_bound, const double& max_bound, const size_t& nb_of_values) :
_min_bound(min_bound),
_max_bound(max_bound),
_nb_of_values(nb_of_values),
vec(std::vector<double>()),
delta((_max_bound-_min_bound)/double(_nb_of_values-1))
{
    if (_min_bound>_max_bound)
    {
        std::stringstream msg;
        msg << "_min_bound = " << _min_bound
            << ", _max_bound = " << _max_bound;
        THROW(__PRETTY_FUNCTION__, VectorOfEquallySpacedNumbersException, msg.str());
    }
    if (_nb_of_values<1)
    {
        THROW(__PRETTY_FUNCTION__, VectorOfEquallySpacedNumbersException, "need at least two values in VectorOfEquallySpacedNumbers");
    }
    if (_nb_of_values>vec.max_size())
    {
        THROW(__PRETTY_FUNCTION__, VectorOfEquallySpacedNumbersException, "too many elements");
    }
    if (_nb_of_values>1)
    {
        vec.reserve(_nb_of_values);
        for (size_t i = 0 ; i < _nb_of_values ; ++i)
        {
            vec.push_back(_min_bound*(double(_nb_of_values)-1)+double(i)*(_max_bound-_min_bound));
            vec.back() /= double(_nb_of_values-1);
        }
    }
    else
    {
        if (min_bound!=max_bound)
        {
            THROW(__PRETTY_FUNCTION__, VectorOfEquallySpacedNumbersException, "If there is only one value, min & max need to be equal");
        }
        delta = 0;
        vec.push_back(min_bound);
    }
}

std::vector<double> VectorOfEquallySpacedNumbers::get() const
{
    return vec;
}

double VectorOfEquallySpacedNumbers::get_delta() const
{
    return delta;
}

size_t VectorOfEquallySpacedNumbers::size() const
{
    return _nb_of_values;
}

double VectorOfEquallySpacedNumbers::get_min() const
{
    return _min_bound;
}

double VectorOfEquallySpacedNumbers::get_max() const
{
    return _max_bound;
}

double VectorOfEquallySpacedNumbers::at(const size_t& i) const
{
    if (_nb_of_values==1) return vec.back();
    if (i>=_nb_of_values)
    {
        THROW(__PRETTY_FUNCTION__, VectorOfEquallySpacedNumbersException, "Index out of range");
    }
    return (_min_bound*(double(_nb_of_values)-1)+double(i)*double(_max_bound-_min_bound))/double(_nb_of_values-1);
}
