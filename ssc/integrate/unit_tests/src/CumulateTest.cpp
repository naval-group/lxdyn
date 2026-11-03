/*
 * CumulateTest.cpp
 *
 * \date 8 oct. 2013, 08:22:10
 * \author cec
 */

#include "CumulateTest.hpp"
#include "ssc/integrate/Cumulate.hpp"

CumulateTest::CumulateTest() : a(ssc::random_data_generator::DataGenerator(22))
{
}

CumulateTest::~CumulateTest()
{
}

void CumulateTest::SetUp()
{
}

void CumulateTest::TearDown()
{
}

TEST_F(CumulateTest, example)
{
//! [CumulateTest example]
    ssc::integrate::Cumulate c;
    c.add(1,1);
    ASSERT_DOUBLE_EQ(0,c.integrate(a.random<double>(),a.random<double>()));
    c.add(2,2);
    ASSERT_DOUBLE_EQ(1.5, c.integrate(1,2));
    c.add(3,2);
    ASSERT_DOUBLE_EQ(1.5, c.integrate(1,2));
    ASSERT_DOUBLE_EQ(2, c.integrate(2,3));
    ASSERT_DOUBLE_EQ(3.5, c.integrate(1,3));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_NO_THROW(c.integrate(a.random<double>().between(1,3),a.random<double>().between(1,3)));
        const double x0 =  a.random<double>().between(1,3);
        ASSERT_DOUBLE_EQ(0, c.integrate(x0,x0));
    }
    c.add(2.5,80);
    ASSERT_DOUBLE_EQ(42.5, c.integrate(1,3));
//! [CumulateTest example]
//! [CumulateTest expected output]
//! [CumulateTest expected output]
}

TEST_F(CumulateTest, zero_outside_bounds)
{
//! [CumulateTest example]
    ssc::integrate::Cumulate c;
    c.add(1,1);
    c.add(2,2);
    ASSERT_DOUBLE_EQ(0, c.integrate(0,1));
}

TEST_F(CumulateTest, bug_detected_in_CumulateModule)
{
    ssc::integrate::Cumulate c;
    c.add(0,0);
    c.add(1,1);
    c.add(2,4);
    c.add(1.5,2.25);
    ASSERT_DOUBLE_EQ(1.3125, c.integrate(0,1.5));
}
