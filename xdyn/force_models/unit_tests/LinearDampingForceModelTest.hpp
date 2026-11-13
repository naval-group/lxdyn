/*
 * LinearDampingForceModelTest.hpp
 *
 *  Created on: Oct 17, 2014
 *      Author: cady
 */

#ifndef LINEARDAMPINGFORCEMODELTEST_HPP_
#define LINEARDAMPINGFORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class LinearDampingForceModelTest : public ::testing::Test {
protected:
    LinearDampingForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* LINEARDAMPINGFORCEMODELTEST_HPP_ */
