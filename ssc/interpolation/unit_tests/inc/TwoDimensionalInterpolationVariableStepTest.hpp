/*
 * TwoDimensionalInterpolationVariableStepTest.hpp
 *
 * \date Dec 17, 2014
 * \author cec
 */

#ifndef TWODIMENSIONALINTERPOLATIONVARIABLESTEPTEST_HPP_
#define TWODIMENSIONALINTERPOLATIONVARIABLESTEPTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class TwoDimensionalInterpolationVariableStepTest : public ::testing::Test
{
    protected:
        TwoDimensionalInterpolationVariableStepTest();
        ~TwoDimensionalInterpolationVariableStepTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* TWODIMENSIONALINTERPOLATIONVARIABLESTEPTEST_HPP_ */
