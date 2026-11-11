/*
 * ParabolicInterpolationTest.hpp
 *
 * \date 19 févr. 2013, 09:55:50
 * \author cec
 */

#ifndef PARABOLICINTERPOLATIONTEST_HPP_
#define PARABOLICINTERPOLATIONTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class ParabolicInterpolationTest : public ::testing::Test
{
    protected:
        ParabolicInterpolationTest();
        ~ParabolicInterpolationTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* PARABOLICINTERPOLATIONTEST_HPP_ */
