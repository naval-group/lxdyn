/*
 * BretschneiderSpectrumTest.hpp
 *
 *  Created on: Jul 30, 2014
 *      Author: cady
 */

#ifndef BRETSCHNEIDERSPECTRUMTEST_HPP_
#define BRETSCHNEIDERSPECTRUMTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class BretschneiderSpectrumTest : public ::testing::Test
{
    protected:
        BretschneiderSpectrumTest();
        ~BretschneiderSpectrumTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* BRETSCHNEIDERSPECTRUMTEST_HPP_ */
