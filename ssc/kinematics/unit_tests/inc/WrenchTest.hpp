/*
 * WrenchTest.hpp
 *
 * \date Apr 24, 2014
 * \author cec
 */

#ifndef WRENCHTEST_HPP_
#define WRENCHTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class WrenchTest : public ::testing::Test
{
    protected:
        WrenchTest();
        ~WrenchTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* WRENCHTEST_HPP_ */
