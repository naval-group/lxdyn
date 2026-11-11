/*
 * ClenshawCurtisTest.hpp
 *
 * \date Dec 9, 2014
 * \author cec
 */


#ifndef CLENSHAWCURTISTEST_HPP_
#define CLENSHAWCURTISTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class ClenshawCurtisTest : public ::testing::Test
{
    protected:
        ClenshawCurtisTest();
        ~ClenshawCurtisTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* CLENSHAWCURTISTEST_HPP_ */
