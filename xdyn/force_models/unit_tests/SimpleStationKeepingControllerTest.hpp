/*
 * SimpleStationKeepingControllerTest.hpp
 *
 *  Created on: Jan 29, 2015
 *      Author: cady
 */

#ifndef SIMPLESTATIONKEEPINGCONTROLLERTEST_HPP_
#define SIMPLESTATIONKEEPINGCONTROLLERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class SimpleStationKeepingControllerTest : public ::testing::Test {
protected:
    SimpleStationKeepingControllerTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* SIMPLESTATIONKEEPINGCONTROLLERTEST_HPP_ */
