/*
 * PiecewiseConstantTest.hpp
 *
 * \date 18 févr. 2013, 12:59:14
 * \author cec
 */

#ifndef PIECEWISECONSTANTTEST_HPP_
#define PIECEWISECONSTANTTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class PiecewiseConstantTest : public ::testing::Test
{
    protected:
        PiecewiseConstantTest();
        ~PiecewiseConstantTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* PIECEWISECONSTANTTEST_HPP_ */
