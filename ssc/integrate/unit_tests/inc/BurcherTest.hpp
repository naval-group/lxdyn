/*
 * BurcherTest.hpp
 *
 * \date Dec 11, 2014
 * \author cec
 */


#ifndef BURCHERTEST_HPP_
#define BURCHERTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class BurcherTest : public ::testing::Test
{
    protected:
        BurcherTest();
        ~BurcherTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* BURCHERTEST_HPP_ */
