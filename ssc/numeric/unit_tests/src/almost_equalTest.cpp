/*
 * almost_equalTest.cpp
 *
 * \date 26 nov. 2013, 08:41:34
 * \author cec
 */

#include "ssc/numeric/almost_equal.hpp"
#include "almost_equalTest.hpp"
#include <cmath>
#include <limits>

almost_equalTest::almost_equalTest() : a(ssc::random_data_generator::DataGenerator(7214222))
{
}

almost_equalTest::~almost_equalTest()
{
}

void almost_equalTest::SetUp()
{
}

void almost_equalTest::TearDown()
{
}

TEST_F(almost_equalTest, can_get_a_number_close_to_another)
{
    std::cout << std::setprecision(20);
    for (size_t i = 0 ; i < 10000 ; ++i)
    {
        const double x = a.random<double>().between(-1000,1000);
        ASSERT_LT(x,one_ULP_more(x));
        ASSERT_LT(one_ULP_less(x),x);
        ASSERT_DOUBLE_EQ(x,one_ULP_less(x));
        ASSERT_DOUBLE_EQ(x,one_ULP_more(x));
    }
}

TEST_F(almost_equalTest, can_get_a_number_not_too_close_to_another)
{
    for (size_t i = 0 ; i < 10000 ; ++i)
    {
        const double x = a.random<double>().between(-1000,1000);
        ASSERT_LT(x,five_ULP_more(x));
        ASSERT_LT(five_ULP_less(x),x);
    }
}

TEST_F(almost_equalTest, example)
{
//! [almost_equalTest example]
    for (size_t i = 0 ; i < 10000 ; ++i)
    {
        const double x = a.random<double>().between(-1000,1000);
        ASSERT_TRUE(almost_equal(x,x));
        ASSERT_TRUE(almost_equal(x,one_ULP_more(x)));
        ASSERT_DOUBLE_EQ(one_ULP_more(x),x);
        ASSERT_FALSE(almost_equal(five_ULP_more(x),x));
        ASSERT_DOUBLE_EQ(one_ULP_less(x),x);
        ASSERT_FALSE(almost_equal(five_ULP_less(x),x));
        ASSERT_TRUE(almost_equal(x,one_ULP_less(x)));
        ASSERT_FALSE(almost_equal(five_ULP_less(x),x));
    }
//! [almost_equalTest example]
//! [almost_equalTest expected output]
//! [almost_equalTest expected output]
}

double almost_equalTest::one_ULP_less(const double x) const
{
    return nextafter(x,-std::numeric_limits<double>::max());
}

double almost_equalTest::one_ULP_more(const double x) const
{
    return nextafter(x,std::numeric_limits<double>::max());
}

double almost_equalTest::five_ULP_less(double x) const
{
    for (size_t i = 0 ; i < 5 ; ++i) x = one_ULP_less(x);
    return x;
}

double almost_equalTest::five_ULP_more(double x) const
{
    for (size_t i = 0 ; i < 5 ; ++i) x = one_ULP_more(x);
    return x;
}
