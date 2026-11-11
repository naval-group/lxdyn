/*
 * vector2stringTest.hpp
 *
 * \date May 5, 2015
 * \author cec
 */


#ifndef VECTOR2STRINGTEST_HPP_
#define VECTOR2STRINGTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class string2vectorTest : public ::testing::Test
{
    protected:
        string2vectorTest();
        ~string2vectorTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* VECTOR2STRINGTEST_HPP_ */
