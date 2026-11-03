/*
 * kahan_sumTest.cpp
 *
 * \date May 19, 2014
 * \author cec
 */

#include "kahan_sumTest.hpp"
#include "ssc/numeric/kahan_sum.hpp"
#include <functional> // std::plus
#include <numeric>    // std::accumulate



using namespace ssc::numeric;

kahan_sumTest::kahan_sumTest() : a(ssc::random_data_generator::DataGenerator(54212))
{
}

kahan_sumTest::~kahan_sumTest()
{
}

void kahan_sumTest::SetUp()
{
}

void kahan_sumTest::TearDown()
{
}

TEST_F(kahan_sumTest, example)
{
//! [kahan_sumTest example]
    std::vector<int> v;
    for (int i = 1 ; i <= 14 ; ++i)  v.push_back(i);
//! [kahan_sumTest example]
//! [kahan_sumTest expected output]
    ASSERT_EQ(105, kahan(v));
//! [kahan_sumTest expected output]
}

TEST_F(kahan_sumTest, example_from_wikipedia)
{
    std::vector<double> v;
    for (size_t i = 1 ; i <= 17 ; ++i) v.push_back(double(i)/10);
    const double naive = std::accumulate (v.begin(),v.end(),0,std::plus<double>());
    const double kahan_sum = kahan(v);
    ASSERT_NEAR(8,naive, 1E-1);
    ASSERT_NEAR(15.3,kahan_sum, 1E-1);
}


