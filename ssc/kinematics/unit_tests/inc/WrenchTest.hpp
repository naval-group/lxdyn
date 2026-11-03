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
        virtual ~WrenchTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* WRENCHTEST_HPP_ */
