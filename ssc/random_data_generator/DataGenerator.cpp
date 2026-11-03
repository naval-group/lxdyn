#include "ssc/random_data_generator/DataGenerator.hpp"
#include "ssc/random_data_generator/sir_rand.h"

#include <iostream>
#include <math.h> // And *not* cmath: otherwise isnan won't work
#include <cstddef> // size_t

#define MAX_RAND_INT 2147483647

#define MIN(a,b) (a<b) ? a : b;
#define MAX(a,b) (a>b) ? a : b;

#if defined(_MSC_VER)
    /* Microsoft Visual Studio. --------------------------------- */
    #include <float.h> // _isnan
    #define isnan _isnan
    #define NAN 0x7F800001
#endif

#if defined(__MINGW32__)
    /*
    With MinGW and GCC, isnan macro is creating some compilation problems

    error: conversion to 'float' from 'int' may alter its value [-Werror=conversion]
    if (isnan((float)min_bound))                                  return NAN;
        ^
    compilation terminated due to -Wfatal-errors.

    To workaround this error, we use default isnan
    macro

    With GCC 6.1.0, this was fixed
    URL related to this bug
    https://sourceforge.net/p/mingw-w64/bugs/481/
    https://github.com/cpputest/cpputest/issues/669
    */
    #undef isnan
    #define isnan(x) (sizeof (x) == sizeof (float) ? __isnanf (x)   \
                    : sizeof (x) == sizeof (double) ? __isnan (x) \
                    : __isnanl (x))
#endif

using namespace ssc::random_data_generator;

template <typename T> bool outside_forbidden_zone(const T& forbidden_min, const T& value_to_test, const T& forbidden_max)
{
    return not((value_to_test>=forbidden_min)&&(value_to_test<=forbidden_max));
}

DataGenerator::DataGenerator(const size_t& seed) : negated(false)
{
    srand((unsigned int)(seed));
}

DataGenerator& DataGenerator::no()
{
    negated = true;
    return *this;
}

double DataGenerator::random_double(const double& a, const double& b) const
{
    const double r = sir_rand_u01();
    return (b-a)*r + a;
}
/*
template <> TypedVectorDataGenerator<double>& TypedVectorDataGenerator<double>::between(const double& t1, const double& t2)
{
    min_bound = t1;
    max_bound = t2;
    return *this;
}

template <> TypedVectorDataGenerator<double>& TypedVectorDataGenerator<double>::greater_than(const double& t)
{
    if (negated)
    {
        max_bound = t;
    }
    else
    {
        min_bound = t;
    }
    return *this;
}
*/

int DataGenerator::random_int(const int& a, const int& b) const
{
    return a + static_cast<int>(sir_rand_u01()*(b-a));
}

size_t DataGenerator::random_size_t(const size_t& a, const size_t& b) const
{
    return a + static_cast<size_t>(floor(sir_rand_u01()*double(b-a)+0.5));
}

float DataGenerator::random_float(const float& a, const float& b) const
{
    return a + static_cast<float>(sir_rand_u01()*(b-a));
}

long DataGenerator::random_long(const long& a, const long& b) const
{
    return a + static_cast<long>(floor(sir_rand_u01()*double(b-a)+0.5));
}

short DataGenerator::random_short(const short& a, const short& b) const
{
    return (short)(a + floor(sir_rand_u01()*(double(b)-double(a))+0.5));
}

bool DataGenerator::random_bool() const
{
    return (sir_rand_u01()>0.5);
}

char DataGenerator::random_char() const
{
    const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    const size_t random_index = (size_t)get_random_number() % (sizeof(alphanum)/sizeof(char) - 1);
    return alphanum[random_index];
}

const std::string DataGenerator::random_string(const size_t& string_length) const
{
    std::string ret;
    ret.reserve(string_length);
    for (size_t i = 0 ; i < string_length ; ++i)
    {
        ret += random_char();
    }
    return ret;
}

const std::string DataGenerator::random_string(const size_t& min_string_length, const size_t& max_string_length) const
{
    const size_t string_length = random_size_t(min_string_length, max_string_length);
    return random_string(string_length);
}


const std::string DataGenerator::random_string() const
{
    return random_string(GET_A_MIN_STRING_LENGTH, GET_A_MAX_STRING_LENGTH);
}

const std::vector<size_t>      DataGenerator::vector_of_random_size_ts(const size_t& n, const size_t& a, const size_t& b) const
{
    std::vector<size_t> ret;
    ret.reserve(n);
    for (size_t i = 0 ; i < n ; ++i)
    {
        ret.push_back(random_size_t(a, b));
    }
    return ret;
}

const std::vector<double>      DataGenerator::vector_of_random_doubles(const size_t& n, const double& a, const double& b) const
{
    std::vector<double> ret;
    ret.reserve(n);
    for (size_t i = 0 ; i < n ; ++i)
    {
        ret.push_back(random_double(a,b));
    }
    return ret;
}

int DataGenerator::get_random_number() const
{
    return static_cast<int>(sir_rand_u01()*MAX_RAND_INT);
}

namespace ssc
{
    namespace random_data_generator
    {
        template <> double ssc::random_data_generator::TypedScalarDataGenerator<double>::get() const
        {
            if (isnan((float)min_bound))                                  return NAN;
            if (isnan((float)max_bound))                                  return NAN;
            if (isnan((float)forbidden_min))                              return NAN;
            if (isnan((float)forbidden_max))                              return NAN;
            if ((forbidden_min==min_bound) && (forbidden_max==max_bound)) return 0;
            const double a = random_double(min_bound, max_bound);
            if (outside_forbidden_zone(forbidden_min, a, forbidden_max))  return a;
            else                                                          return get();
        }
    }
}

template <> double ssc::random_data_generator::TypedScalarDataGenerator<double>::operator()()
{
    double tmp = get();
    min_bound = get_min_bound<double>();
    max_bound = get_max_bound<double>();
    negated = false;
    return tmp;
}

namespace ssc
{
    namespace random_data_generator
    {
        template <> size_t TypedScalarDataGenerator<size_t>::get() const
        {
            if ((forbidden_min==min_bound) && (forbidden_max==max_bound)) return 0;
            const size_t a = random_size_t(min_bound, max_bound);
            if (outside_forbidden_zone(forbidden_min, a, forbidden_max)) return a;
            else                                                         return get();
        }

        template <> float TypedScalarDataGenerator<float>::get() const
        {

            if ((forbidden_min==min_bound) && (forbidden_max==max_bound)) return 0;
            const float a = random_float(min_bound, max_bound);
            if (outside_forbidden_zone(forbidden_min, a, forbidden_max)) return a;
            else                                                         return get();
        }

        template <> int TypedScalarDataGenerator<int>::get() const
        {

            if ((forbidden_min==min_bound) && (forbidden_max==max_bound)) return 0;
            const int a = (int)random_double(min_bound, max_bound);
            if (outside_forbidden_zone(forbidden_min, a, forbidden_max)) return a;
            else                                                         return get();
        }


        template <> bool TypedScalarDataGenerator<bool>::get() const
        {
            return random_bool();
        }

        template <> std::string TypedScalarDataGenerator<std::string>::get() const
        {
            return random_string();
        }

        template<> double get_min_bound<double>()
        {
            return -1e10;
        }

        template<> double get_max_bound<double>()
        {
            return 1e10;
        }

        template<> size_t get_min_bound<size_t>()
        {
            return 0;
        }

        template<> size_t get_max_bound<size_t>()
        {
            return (size_t)-1;
        }
    }
}
