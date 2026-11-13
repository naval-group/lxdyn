/*
 * ManeuveringForceModelTest.hpp
 *
 *  Created on: Jan 19, 2015
 *      Author: cady
 */

#ifndef MANEUVERINGFORCEMODELTEST_HPP_
#define MANEUVERINGFORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class ManeuveringForceModelTest : public ::testing::Test {
protected:
    ManeuveringForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* MANEUVERINGFORCEMODELTEST_HPP_ */
