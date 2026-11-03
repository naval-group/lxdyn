/*
 * PiecewiseConstantVariableStep.hpp
 *
 * \date 11 avr. 2013, 08:44:04
 * \author cec
 */

#ifndef PIECEWISECONSTANTVARIABLESTEPTEST_HPP_
#define PIECEWISECONSTANTVARIABLESTEPTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class PiecewiseConstantVariableStepTest : public ::testing::Test
{
    protected:
        PiecewiseConstantVariableStepTest();
        virtual ~PiecewiseConstantVariableStepTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* PIECEWISECONSTANTVARIABLESTEPTEST_HPP_ */
