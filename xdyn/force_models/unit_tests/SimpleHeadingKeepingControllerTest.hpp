/*
 * SimpleTrackKeepingControllerTest.hpp
 *
 *  Created on: Jan 15, 2015
 *      Author: cady
 */

#ifndef SIMPLETRACKKEEPINGCONTROLLERTEST_HPP_
#define SIMPLETRACKKEEPINGCONTROLLERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class SimpleHeadingKeepingControllerTest : public ::testing::Test {
protected:
    SimpleHeadingKeepingControllerTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* SIMPLETRACKKEEPINGCONTROLLERTEST_HPP_ */
