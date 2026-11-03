/*
 * PiecewiseConstantTest.cpp
 *
 * \date 18 févr. 2013, 12:59:23
 * \author cec
 */

#include "PiecewiseConstantTest.hpp"
#include "ssc/interpolation/PiecewiseConstant.hpp"
#include "ssc/interpolation/PiecewiseConstantException.hpp"
#include "ssc/interpolation/InterpolatorException.hpp"

using namespace ssc::interpolation;

PiecewiseConstantTest::PiecewiseConstantTest() : a(ssc::random_data_generator::DataGenerator(22338888))
{
}

PiecewiseConstantTest::~PiecewiseConstantTest()
{
}

void PiecewiseConstantTest::SetUp()
{
}

void PiecewiseConstantTest::TearDown()
{
}

TEST_F(PiecewiseConstantTest, example)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
//! [PiecewiseConstantTest example]
    const size_t n = a.random<size_t>().greater_than(1).but().no().greater_than(1000);
    const std::vector<double> vals = a.random_vector_of<double>().of_size(n);
    const double xmin = a.random<double>();
    const double xmax = a.random<double>().greater_than(xmin);
    PiecewiseConstant cst(xmin, xmax, vals);
//! [PiecewiseConstantTest example]
//! [PiecewiseConstantTest expected output]
    const double val = a.random<double>().between(xmin,xmax);
    cst.f(val);
//! [PiecewiseConstantTest expected output]
    }
}

TEST_F(PiecewiseConstantTest, should_throw_if_y_has_less_than_two_elements)
{
    const double xmin = a.random<double>();
    const double xmax = a.random<double>().greater_than(xmin);
    ASSERT_THROW(PiecewiseConstant(xmin, xmax, a.random_vector_of<double>().of_size(1)), PiecewiseConstantException);
    ASSERT_THROW(PiecewiseConstant(xmin, xmax, std::vector<double>()), InterpolatorException);
}

TEST_F(PiecewiseConstantTest, should_throw_if_xmin_greater_than_xmax)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double xmax = a.random<double>();
        const double xmin = a.random<double>().greater_than(xmax);
        const size_t n = a.random<size_t>().greater_than(1).but().no().greater_than(1000);
        ASSERT_THROW(PiecewiseConstant(xmin, xmax, a.random_vector_of<double>().of_size(n)), InterpolatorException);
    }
}

TEST_F(PiecewiseConstantTest, should_not_throw_if_xmin_equals_xmax)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double xmin = a.random<double>();
        const double xmax = xmin;
        const size_t n = a.random<size_t>().greater_than(1).but().no().greater_than(1000);
        ASSERT_NO_THROW(PiecewiseConstant(xmin, xmax, a.random_vector_of<double>().of_size(n)));
    }
}


TEST_F(PiecewiseConstantTest, second_derivative_should_be_zero)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const size_t n = a.random<size_t>().greater_than(1).but().no().greater_than(1000);
        const std::vector<double> vals = a.random_vector_of<double>().of_size(n);
        const double xmin = a.random<double>();
        const double xmax = a.random<double>().greater_than(xmin);
        PiecewiseConstant cst(xmin, xmax, vals);
        const double val = a.random<double>().between(xmin,xmax);
        ASSERT_EQ(0, cst.df(val,2));
    }
}

TEST_F(PiecewiseConstantTest, first_derivative_should_be_zero)
{
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const size_t n = a.random<size_t>().greater_than(1).but().no().greater_than(1000);
        const std::vector<double> vals = a.random_vector_of<double>().of_size(n);
        const double xmin = a.random<double>();
        const double xmax = a.random<double>().greater_than(xmin);
        PiecewiseConstant cst(xmin, xmax, vals);
        const double val = a.random<double>().between(xmin,xmax);
        ASSERT_EQ(0, cst.df(val));
    }
}

TEST_F(PiecewiseConstantTest, computed_values_should_be_correct)
{
    std::vector<double> v;
    v.push_back(3);v.push_back(6);v.push_back(5);v.push_back(8);v.push_back(7);v.push_back(4);v.push_back(5);v.push_back(6);v.push_back(9);v.push_back(78);
    PiecewiseConstant pc(0, 10, v);
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(0,1);
        ASSERT_DOUBLE_EQ(3,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(1,2);
        ASSERT_DOUBLE_EQ(6,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(2,3);
        ASSERT_DOUBLE_EQ(5,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(3,4);
        ASSERT_DOUBLE_EQ(8,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(4,5);
        ASSERT_DOUBLE_EQ(7,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(5,6);
        ASSERT_DOUBLE_EQ(4,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(6,7);
        ASSERT_DOUBLE_EQ(5,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(7,8);
        ASSERT_DOUBLE_EQ(6,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(8,9);
        ASSERT_DOUBLE_EQ(9,pc.f(x));
    }
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(9,10);
        ASSERT_DOUBLE_EQ(78,pc.f(x));
    }
}

TEST_F(PiecewiseConstantTest, bug_detected_in_EONAV)
{
    std::vector<double> v;
    v.push_back(3);v.push_back(6);v.push_back(5);v.push_back(8);v.push_back(7);v.push_back(4);v.push_back(5);v.push_back(6);v.push_back(9);v.push_back(72);
    PiecewiseConstant pc(0, 10, v);

    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().greater_than(10);
        ASSERT_DOUBLE_EQ(72, pc.f(x));
    }
}
