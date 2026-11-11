#ifndef VELOCITYTEST_HPP_
#define VELOCITYTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class VelocityTest : public ::testing::Test
{
    protected:
        VelocityTest();
        ~VelocityTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* VELOCITYTEST_HPP_ */
