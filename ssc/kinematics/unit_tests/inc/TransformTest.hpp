#ifndef TRANSFORMTEST_HPP_
#define TRANSFORMTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class TransformTest : public ::testing::Test
{
    protected:
        TransformTest();
        virtual ~TransformTest();
        virtual void SetUp();
        virtual void TearDown();

        ssc::random_data_generator::DataGenerator a;
};

#endif  /* TRANSFORMTEST_HPP_ */
