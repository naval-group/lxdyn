/*
 * LinearInterpolationTest.cpp
 *
 * \date 8 nov. 2012, 14:31:52
 * \author cec
 */

#include "LinearInterpolationTest.hpp"
#include "ssc/interpolation/LinearInterpolation.hpp"

using namespace ssc::interpolation;

LinearInterpolationTest::LinearInterpolationTest() : a(ssc::random_data_generator::DataGenerator(23))
{
}

LinearInterpolationTest::~LinearInterpolationTest()
{
}

void LinearInterpolationTest::SetUp()
{
}

void LinearInterpolationTest::TearDown()
{
}

TEST_F(LinearInterpolationTest, example)
{
//! [LinearInterpolationTest example]
    std::vector<double> v;
    v.push_back(0);v.push_back(1);v.push_back(2);v.push_back(3);v.push_back(4);v.push_back(5);v.push_back(6);v.push_back(7);v.push_back(8);v.push_back(9);v.push_back(10);
    LinearInterpolation interpolate(0,10,v);
//! [LinearInterpolationTest example]
//! [LinearInterpolationTest expected output]
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double val = a.random<double>().between(0,10);
        ASSERT_DOUBLE_EQ(val, interpolate.f(val));
    }
//! [LinearInterpolationTest expected output]
}

TEST_F(LinearInterpolationTest, f_should_work_properly)
{
    std::vector<double> v;
    v.push_back(10);
    v.push_back(0);
    LinearInterpolation interpolate(0,10,v);
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double val = a.random<double>().between(0,10);
        ASSERT_DOUBLE_EQ(10-val, interpolate.f(val));
    }
    v.clear();
    v.push_back(1);v.push_back(10);v.push_back(40);v.push_back(98);v.push_back(2);v.push_back(3);v.push_back(7);v.push_back(0);v.push_back(8);v.push_back(10);v.push_back(11);
    LinearInterpolation interpolate2(0,10,v);
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double val = a.random<double>().between(4,5);
        ASSERT_DOUBLE_EQ(val-2, interpolate2.f(val));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double val = a.random<double>().between(9,10);
        ASSERT_DOUBLE_EQ(val+1, interpolate2.f(val));
    }
}

TEST_F(LinearInterpolationTest, df_should_work_properly)
{
    std::vector<double> v;
    v.push_back(10);
    v.push_back(0);
    LinearInterpolation interpolate(0,10,v);
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double val = a.random<double>().between(0,10);
        ASSERT_DOUBLE_EQ(-1, interpolate.df(val));
    }
}

TEST_F(LinearInterpolationTest, d2f_should_work_properly)
{
    const size_t n = a.random<size_t>().no().greater_than(30).but().greater_than(0);
    LinearInterpolation interpolate(0,10,a.random_vector_of<double>().of_size(n));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double val = a.random<double>();
        ASSERT_EQ(0, interpolate.df(val,2));
    }
}

TEST_F(LinearInterpolationTest, bug_detected_in_EONAV_PiecewiseLinearFunctor)
{
    std::vector<double> v;
    v.push_back(3);v.push_back(6);v.push_back(5);v.push_back(8);v.push_back(7);v.push_back(4);v.push_back(5);v.push_back(6);v.push_back(9);v.push_back(72);v.push_back(-56);
    LinearInterpolation pl(0, 10, v);
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(3,pl.df(a.random<double>().between(0,1)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(-1,pl.df(a.random<double>().between(1,2)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(3,pl.df(a.random<double>().between(2,3)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(-1,pl.df(a.random<double>().between(3,4)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(-3,pl.df(a.random<double>().between(4,5)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(1,pl.df(a.random<double>().between(5,6)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(1,pl.df(a.random<double>().between(6,7)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(3,pl.df(a.random<double>().between(7,8)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(63,pl.df(a.random<double>().between(8,9)));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(-128,pl.df(a.random<double>().between(9,10)));
    }
}
