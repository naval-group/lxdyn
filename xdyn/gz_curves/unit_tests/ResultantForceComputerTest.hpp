/*
 * ResultantForceComputerTest.hpp
 *
 *  Created on: Feb 9, 2015
 *      Author: cady
 */

#ifndef RESULTANTFORCECOMPUTERTEST_HPP_
#define RESULTANTFORCECOMPUTERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"
#include "xdyn/core/Sim.hpp"

class ResultantForceComputerTest : public ::testing::Test {
protected:
    ResultantForceComputerTest();
    ssc::random_data_generator::DataGenerator a;
    Sim& sim;
};

#endif /* RESULTANTFORCECOMPUTERTEST_HPP_ */
