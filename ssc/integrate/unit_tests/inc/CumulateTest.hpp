/*
 * CumulateTest.hpp
 *
 * \date 8 oct. 2013, 08:21:58
 * \author cec
 */

#ifndef CUMULATETEST_HPP_
#define CUMULATETEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class CumulateTest : public ::testing::Test
{
    protected:
        CumulateTest();
        ~CumulateTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* CUMULATETEST_HPP_ */
