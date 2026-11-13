/*
 * MMGPropellerForceModelTest.hpp
 *
 *  Created on: Oct 3, 2024
 *      Author: fbickert
 */

#ifndef MMGPROPELLERFORCEMODELTEST_HPP_
#define MMGPROPELLERFORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"
#include "xdyn/core/EnvironmentAndFrames.hpp"

class MMGPropellerForceModelTest : public ::testing::Test {
protected:
    MMGPropellerForceModelTest();
    ssc::random_data_generator::DataGenerator a;
    EnvironmentAndFrames get_env();
};

#endif /* MMGPROPELLERFORCEMODELTEST_HPP_ */
