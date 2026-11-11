/*
 * EkmanUWCurrentTest.hpp
 */

#ifndef EKMANUWCURRENTTEST_HPP_
#define EKMANUWCURRENTTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class EkmanUWCurrentTest : public ::testing::Test
{
    protected:
        EkmanUWCurrentTest();
        ~EkmanUWCurrentTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif /* EKMANUWCURRENTTEST_HPP_ */
