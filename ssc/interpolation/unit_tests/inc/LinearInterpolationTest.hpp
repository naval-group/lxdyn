/*
 * LinearInterpolationTest.hpp
 *
 * \date 8 nov. 2012, 14:31:45
 * \author cec
 */

#ifndef LINEARINTERPOLATIONTEST_HPP_
#define LINEARINTERPOLATIONTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class LinearInterpolationTest : public ::testing::Test
{
    protected:
        LinearInterpolationTest();
        ~LinearInterpolationTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* LINEARINTERPOLATIONTEST_HPP_ */
