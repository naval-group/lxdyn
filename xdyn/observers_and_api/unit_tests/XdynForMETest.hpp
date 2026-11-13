/*
 * XdynForMETest.hpp
 *
 *  Created on: Nov 15, 2018
 *      Author: cady
 */

#ifndef OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORMETEST_HPP_
#define OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORMETEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class XdynForMETest : public ::testing::Test {
protected:
    XdynForMETest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORMETEST_HPP_ */
