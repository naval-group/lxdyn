/*
 * TrapezoidalIntegrationTest.hpp
 *
 * \date 23 avr. 2013, 14:46:13
 * \author cec
 */

#ifndef TRAPEZOIDALINTEGRATIONTEST_HPP_
#define TRAPEZOIDALINTEGRATIONTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class TrapezoidalIntegrationTest : public ::testing::Test
{
    protected:
        TrapezoidalIntegrationTest();
        virtual ~TrapezoidalIntegrationTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* TRAPEZOIDALINTEGRATIONTEST_HPP_ */
