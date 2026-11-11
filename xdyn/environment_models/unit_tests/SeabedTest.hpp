/*
 * SeabedTest.hpp
 */

#ifndef SEABEDTEST_HPP_
#define SEABEDTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>
#include "Seabed.hpp"

class SeabedTest : public ::testing::Test
{
    protected:
        SeabedTest();
        ~SeabedTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif /* SEABEDTEST_HPP_ */
