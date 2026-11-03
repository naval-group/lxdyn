/*
 * RectangleTest.hpp
 *
 * \date Nov 28, 2014
 * \author cec
 */

#ifndef RECTANGLETEST_HPP_
#define RECTANGLETEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class RectangleTest : public ::testing::Test
{
    protected:
        RectangleTest();
        virtual ~RectangleTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* RECTANGLETEST_HPP_ */
