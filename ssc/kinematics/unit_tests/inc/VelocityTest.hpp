#ifndef VELOCITYTEST_HPP_
#define VELOCITYTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class VelocityTest : public ::testing::Test
{
    protected:
        VelocityTest();
        virtual ~VelocityTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* VELOCITYTEST_HPP_ */
