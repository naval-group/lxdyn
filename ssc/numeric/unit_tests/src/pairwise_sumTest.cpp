/*
 * pairwise_sumTest.cpp
 *
 * \date May 19, 2014
 * \author cec
 */

#include "pairwise_sumTest.hpp"

#include "ssc/numeric/pairwise_sum.hpp"

using namespace ssc::numeric;

pairwise_sumTest::pairwise_sumTest() : a(ssc::random_data_generator::DataGenerator(451212))
{
}

pairwise_sumTest::~pairwise_sumTest()
{
}

void pairwise_sumTest::SetUp()
{
}

void pairwise_sumTest::TearDown()
{
}

TEST_F(pairwise_sumTest, example0)
{
    const std::vector<int> v;
    ASSERT_EQ(0, pairwise(v));
}

TEST_F(pairwise_sumTest, example1)
{
    const std::vector<int> v = {456};
    ASSERT_EQ(456, pairwise(v));
}

TEST_F(pairwise_sumTest, example2)
{
    const std::vector<int> v = {1,2};
    ASSERT_EQ(3, pairwise(v));
}

TEST_F(pairwise_sumTest, example3)
{
    const std::vector<int> v = {1,2,3};
    ASSERT_EQ(6, pairwise(v));
}

TEST_F(pairwise_sumTest, example4)
{
    const std::vector<int> v = {1,2,3,4};
    ASSERT_EQ(10, pairwise(v));
}

TEST_F(pairwise_sumTest, example5)
{
    const std::vector<int> v = {1,2,3,4,5};
    ASSERT_EQ(15, pairwise(v));
}

TEST_F(pairwise_sumTest, example14)
{
    const std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
    ASSERT_EQ(105, pairwise(v));
}

TEST_F(pairwise_sumTest, example_with_index)
{
    const std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
    const std::vector<size_t> idx = {0,1,2,3};
    ASSERT_EQ(10, pairwise(idx,v));
}
