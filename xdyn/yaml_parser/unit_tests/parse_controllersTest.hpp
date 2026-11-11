/*
 * parse_controllersTest.hpp
 *
 *  Created on: Fev 10, 2021
 *      Author: lincker
 */

#ifndef PARSE_CONTROLLERSTEST_HPP_
#define PARSE_CONTROLLERSTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class parse_controllersTest : public ::testing::Test
{
    protected:
        parse_controllersTest();
        ~parse_controllersTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* PARSE_CONTROLLERSTEST_HPP_ */
