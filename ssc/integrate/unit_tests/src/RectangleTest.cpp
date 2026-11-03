/*
 * RectangleTest.cpp
 *
 * \date Nov 28, 2014
 * \author cec
 */

#include "RectangleTest.hpp"
#include "ssc/integrate/Rectangle.hpp"
#include "ssc/macros/extra_test_assertions.hpp"
#include "ssc/macros/test_macros.hpp"
#define EPS 1E-3

RectangleTest::RectangleTest() : a(ssc::random_data_generator::DataGenerator(2122))
{
}

RectangleTest::~RectangleTest()
{
}

void RectangleTest::SetUp()
{
}

void RectangleTest::TearDown()
{
}

TEST_F(RectangleTest, should_be_able_to_integrate_a_constant)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const double c = a.random<double>();
        ssc::integrate::Rectangle integrator([c](const double& x)->double{(void)x;return c;});
        const double xa = a.random<double>().between(-100,100);
        const double xb = a.random<double>().between(xa,100);
        ASSERT_SMALL_RELATIVE_ERROR(integrator.integrate_f(xa,xb,EPS), (xb-xa)*c, EPS);
    }
}

