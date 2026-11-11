/*
 * TwoDimensionalSplinesTest.hpp
 *
 * \date 13 nov. 2012, 16:48:41
 * \author cec
 */

#ifndef TWODIMENSIONALSPLINESTEST_HPP_
#define TWODIMENSIONALSPLINESTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class TwoDimensionalSplinesTest : public ::testing::Test
{
    protected:
        TwoDimensionalSplinesTest();
        ~TwoDimensionalSplinesTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* TWODIMENSIONALSPLINESTEST_HPP_ */
