/*
 * parse_time_seriesTest.hpp
 *
 *  Created on: Oct 22, 2014
 *      Author: cady
 */

#ifndef PARSE_TIME_SERIESTEST_HPP_
#define PARSE_TIME_SERIESTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class parse_time_seriesTest : public ::testing::Test
{
    protected:
        parse_time_seriesTest();
        ~parse_time_seriesTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* PARSE_TIME_SERIESTEST_HPP_ */
