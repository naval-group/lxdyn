/*
 * VectorOfEquallySpacedNumbersTest.hpp
 *
 * \date 31 juil. 2012, 09:07:04
 * \author cec
 */

#ifndef VECTOROFEQUALLYSPACEDNUMBERSTEST_HPP_
#define VECTOROFEQUALLYSPACEDNUMBERSTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class VectorOfEquallySpacedNumbersTest : public ::testing::Test
{
    protected:
        VectorOfEquallySpacedNumbersTest();
        virtual ~VectorOfEquallySpacedNumbersTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;
        const double eps;

};

#endif  /* VECTOROFEQUALLYSPACEDNUMBERSTEST_HPP_ */
