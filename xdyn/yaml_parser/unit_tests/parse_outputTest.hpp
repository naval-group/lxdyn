/*
 * parse_outputTest.hpp
 *
 *  Created on: Jan 13, 2015
 *      Author: cady
 */

#ifndef PARSE_OUTPUTTEST_HPP_
#define PARSE_OUTPUTTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class parse_outputTest : public ::testing::Test
{
    protected:
        parse_outputTest();
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* PARSE_OUTPUTTEST_HPP_ */
