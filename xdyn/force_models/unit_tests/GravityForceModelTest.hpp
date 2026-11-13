/*
 * GravityForceModelTest.hpp
 *
 *  Created on: Jun 17, 2014
 *      Author: cady
 */

#ifndef GRAVITYFORCEMODELTEST_HPP_
#define GRAVITYFORCEMODELTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class GravityForceModelTest : public ::testing::Test {
protected:
    GravityForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* GRAVITYFORCEMODELTEST_HPP_ */
