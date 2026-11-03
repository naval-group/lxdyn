#include "random_data_generator_tests.hpp"
#include <set>
#include <math.h> // And *not* cmath: otherwise isnan won't work

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

#define NB_OF_TRIALS 100

using namespace ssc::random_data_generator;

void DataGeneratorTests::SetUp()
{
}

void DataGeneratorTests::TearDown()
{
}

TEST_F(DataGeneratorTests, should_have_a_random_data_generator_for_doubles)
{
    std::set<double> S;
    for (size_t i =0 ; i < NB_OF_TRIALS ; ++i)
    {
        S.clear();
        const size_t nb_of_values_to_generate = a.random<size_t>().between(1,10);
        for (size_t j = 0 ; j < nb_of_values_to_generate ; ++j)
        {
            S.insert(a.random<double>());
        }
        ASSERT_EQ(nb_of_values_to_generate, S.size());
    }
}

TEST_F(DataGeneratorTests, should_have_a_random_data_generator_for_vector_of_doubles)
{
    std::set<std::vector<double> > S;
    for (size_t i =0 ; i < NB_OF_TRIALS ; ++i)
    {
        S.clear();
        const size_t nb_of_values_to_generate = a.random<size_t>().between(1,5);
        for (size_t j = 0 ; j < nb_of_values_to_generate ; ++j)
        {
            const std::vector<double> v = a.random_vector_of<double>().of_size(3);
            ASSERT_FALSE(v.empty());
            S.insert(v);
        }
        ASSERT_EQ(nb_of_values_to_generate, S.size());
    }

}

TEST_F(DataGeneratorTests, should_be_able_to_generate_doubles_greater_than_a_value)
{
    std::vector<double> v(NB_OF_TRIALS);
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const double lower_bound = a.random<double>();
        ASSERT_GE(a.random<double>().greater_than(lower_bound)(), lower_bound);
    }
}

TEST_F(DataGeneratorTests, should_be_able_to_generate_doubles_no_greater_than_a_value)
{
    std::vector<double> v(NB_OF_TRIALS);
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const double upper_bound = a.random<double>();
        ASSERT_LE(a.random<double>().no().greater_than(upper_bound)(), upper_bound);
    }
}

TEST_F(DataGeneratorTests, should_be_able_to_generate_a_vector_of_doubles_greater_than_a_value)
{
    std::vector<double> v(NB_OF_TRIALS);
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const double lower_bound = a.random<double>();
        const size_t n = a.random<size_t>().no().greater_than(20);
        std::vector<double> v = (std::vector<double>)a.random_vector_of<double>().of_size(n).greater_than(lower_bound);
        for (std::vector<double>::const_iterator it = v.begin() ; it != v.end() ; ++it)
        {
            ASSERT_GE(*it, lower_bound);
        }
    }
}

TEST_F(DataGeneratorTests, should_be_able_to_generate_a_vector_of_doubles_no_greater_than_a_value)
{
    std::vector<double> v(NB_OF_TRIALS);
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const double upper_bound = a.random<double>();
        const size_t n = a.random<size_t>().no().greater_than(20);
        const std::vector<double> v = a.random_vector_of<double>().of_size(n).no().greater_than(upper_bound);
        ASSERT_EQ(n,v.size());
        for (std::vector<double>::const_iterator it = v.begin() ; it != v.end() ; ++it)
        {
            ASSERT_LE(*it, upper_bound);
        }
    }
}

TEST_F(DataGeneratorTests, should_be_able_to_generate_doubles_between_two_values)
{
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const double lower_bound = a.random<double>();
        const double upper_bound = a.random<double>().greater_than(lower_bound);
        ASSERT_LE(a.random<double>().between(lower_bound,upper_bound)(), upper_bound);
        ASSERT_GE(a.random<double>().between(lower_bound,upper_bound)(), lower_bound);
    }
}

TEST_F(DataGeneratorTests, should_be_able_to_generate_doubles_outside_an_interval)
{
    std::set<double> generated_numbers;
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const double number1 = a.random<double>().outside(2.1,2.9).but().between(2,3);
        const double number2 = a.random<double>().between(2,3).but().outside(2.1,2.9);
        ASSERT_LT(number1, 3);
        ASSERT_GT(number1, 2);
        ASSERT_FALSE((number1>=2.1)&&(number1<=2.9));
        ASSERT_LT(number2, 3);
        ASSERT_GT(number2, 2);
        ASSERT_FALSE((number2>=2.1)&&(number2<=2.9));
        generated_numbers.insert(number1);
        generated_numbers.insert(number2);
    }
    ASSERT_EQ(generated_numbers.size(), 2*NB_OF_TRIALS);
}

TEST_F(DataGeneratorTests, should_return_zero_if_forbidden_interval_matches_allowed_interval)
{
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const double min_bound = a.random<double>();
        const double max_bound = a.random<double>().greater_than(min_bound);
        ASSERT_EQ(a.random<double>().between(min_bound,max_bound).but().outside(min_bound,max_bound)(),0);
    }
}



TEST_F(DataGeneratorTests, should_be_able_to_generate_size_ts_for_all_but_one_value)
{
    for (size_t i = 0 ; i < NB_OF_TRIALS; ++i)
    {
        const size_t lower_bound = a.random<size_t>().between(1,5);
        const size_t upper_bound = a.random<size_t>().between(1,5).greater_than(lower_bound);
        const size_t forbidden = a.random<size_t>().between(lower_bound, upper_bound);
        const size_t number = a.random<size_t>().between(lower_bound, upper_bound).but_not(forbidden);
        ASSERT_NE(number, forbidden);
    }
}

TEST_F(DataGeneratorTests, should_be_able_to_generate_a_random_double)
{
    std::set<double> generated_numbers;
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        generated_numbers.insert(a.random<bool>());
    }
    ASSERT_EQ(generated_numbers.size(), 2);
}

TEST_F(DataGeneratorTests, should_be_able_to_generate_a_vector_of_doubles_between_two_values)
{
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const double lower_bound = a.random<double>();
        const double upper_bound = a.random<double>().greater_than(lower_bound);
        std::vector<double> v = a.random_vector_of<double>().between(lower_bound,upper_bound);
        for (std::vector<double>::const_iterator it = v.begin() ; it != v.end() ; ++it)
        {
            ASSERT_LE(*it, upper_bound);
            ASSERT_GE(*it, lower_bound);
        }
    }
}

TEST_F(DataGeneratorTests, bug_detected_in_PiecewiseConstantTest)
{
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const size_t random_double_between_2_and_1000 = a.random<size_t>().greater_than(1).but().no().greater_than(1000);
        ASSERT_LE(random_double_between_2_and_1000, 1000);
        ASSERT_GT(random_double_between_2_and_1000, 1);
    }
}

TEST_F(DataGeneratorTests, bug_detected_in_EONAV)
{
    for (size_t i = 0 ; i < NB_OF_TRIALS ; ++i)
    {
        const size_t n = a.random<size_t>().greater_than(1).but().no().greater_than(10000);
        a.random_vector_of<double>().of_size(n);
        a.random<double>();
        ASSERT_NE(1,n);
    }
}

TEST_F(DataGeneratorTests, crash_detected_in_EONAV_when_one_of_the_interval_bounds_is_nan)
{
    const double x = a.random<double>();
    ASSERT_TRUE(isnan((float)a.random<double>().between(x,NAN)()));
    ASSERT_TRUE(isnan((float)a.random<double>().between(NAN,x)()));
    ASSERT_TRUE(isnan((float)a.random<double>().outside(x,NAN)()));
    ASSERT_TRUE(isnan((float)a.random<double>().outside(NAN,x)()));
}

TEST_F(DataGeneratorTests, bug_in_greater_than_detected_in_EONAV)
{
    std::vector<double> vector_of_doubles;
    const size_t N = vector_of_doubles.max_size();
    const size_t generated = a.random<size_t>().greater_than(N);
    ASSERT_LT(N, generated);
}
