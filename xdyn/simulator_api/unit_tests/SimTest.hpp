/*
 * SimTest.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#ifndef SIMTEST_HPP_
#define SIMTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"
#include "xdyn/mesh/GeometricTypes3d.hpp"

class SimTest : public ::testing::Test {
protected:
    SimTest();
    ssc::random_data_generator::DataGenerator a;
    const VectorOfVectorOfPoints& test_ship_stl;
};

#endif /* SIMTEST_HPP_ */
