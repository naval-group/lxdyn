/*
 * parse_controllersTest.hpp
 *
 *  Created on: Fev 10, 2021
 *      Author: lincker
 */

#ifndef PARSE_CONTROLLERSTEST_HPP_
#define PARSE_CONTROLLERSTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class parse_controllersTest : public ::testing::Test {
protected:
    parse_controllersTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* PARSE_CONTROLLERSTEST_HPP_ */
