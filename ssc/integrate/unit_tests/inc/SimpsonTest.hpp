/*
 * SimpsonTest.hpp
 *
 * \date 23 avr. 2013, 15:37:56
 * \author cec
 */

#ifndef SIMPSONTEST_HPP_
#define SIMPSONTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class SimpsonTest : public ::testing::Test
{
    protected:
        SimpsonTest();
        ~SimpsonTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* SIMPSONTEST_HPP_ */
