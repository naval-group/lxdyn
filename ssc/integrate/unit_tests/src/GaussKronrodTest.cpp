/*
 * QuadPackTest.cpp
 *
 * \date 5 juil. 2013, 13:32:10
 * \author cec
 */

#include "GaussKronrodTest.hpp"
#include "ssc/integrate/GaussKronrod.hpp"
#include "ssc/macros/extra_test_assertions.hpp"

GaussKronrodTest::GaussKronrodTest() : a(ssc::random_data_generator::DataGenerator(2))
{
}

GaussKronrodTest::~GaussKronrodTest()
{
}

void GaussKronrodTest::SetUp()
{
}

void GaussKronrodTest::TearDown()
{
}



TEST_F(GaussKronrodTest, example)
{
    //! [QuadPackTest example]
    ssc::integrate::GaussKronrod integrator([](double x){return x*x;});
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double a_ = a.random<double>().between(-10,0);
        const double b_ = a.random<double>().between(0,10);
        ASSERT_SMALL_RELATIVE_ERROR((b_*b_*b_-a_*a_*a_)/3.,integrator.integrate_f(a_,b_),1E-10);
    }
    //! [QuadPackTest example]
}

TEST_F(GaussKronrodTest, sine_squared_plus_cosine_squared_should_equal_one)
{
    ssc::integrate::GaussKronrod integrator([](double x){return sin(x)*sin(x)+cos(x)*cos(x);});
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double a_ = a.random<double>().between(-10,0);
        const double b_ = a.random<double>().between(0,10);
        ASSERT_DOUBLE_EQ(b_-a_,integrator.integrate_f(a_,b_));
    }
}



