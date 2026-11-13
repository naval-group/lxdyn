/*
 * LinearHydrostaticForceModelTest.hpp
 *
 *  Created on: Aug 12, 2015
 *      Author: cady
 */

#ifndef LINEARHYDROSTATICFORCEMODELTEST_HPP_
#define LINEARHYDROSTATICFORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class LinearHydrostaticForceModelTest : public ::testing::Test {
protected:
    LinearHydrostaticForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* LINEARHYDROSTATICFORCEMODELTEST_HPP_ */
