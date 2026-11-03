/*
 * SplineTest.hpp
 *
 * \date 8 juin 2012, 17:07:01
 * \author cec
 */

#ifndef SPLINETEST_HPP_
#define SPLINETEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class SplinesTest : public ::testing::Test
{
    protected:
        SplinesTest();
        virtual ~SplinesTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* SPLINETEST_HPP_ */
