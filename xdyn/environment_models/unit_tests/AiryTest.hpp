/*
 * AiryTest.hpp
 *
 *  Created on: Aug 1, 2014
 *      Author: cady
 */

#ifndef AIRYTEST_HPP_
#define AIRYTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class AiryTest : public ::testing::Test
{
    protected:
        AiryTest();
        ~AiryTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* AIRYTEST_HPP_ */
