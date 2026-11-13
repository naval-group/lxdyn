/*
 * ClosingFacetComputerTest.hpp
 *
 *  Created on: Feb 27, 2015
 *      Author: cady
 */

#ifndef CLOSINGFACETCOMPUTERTEST_HPP_
#define CLOSINGFACETCOMPUTERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class ClosingFacetComputerTest : public ::testing::Test {
protected:
    ClosingFacetComputerTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* CLOSINGFACETCOMPUTERTEST_HPP_ */
