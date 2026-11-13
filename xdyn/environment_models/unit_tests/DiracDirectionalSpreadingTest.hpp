/*
 * DiracDirectionalSpreadingTest.hpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cady
 */

#ifndef DIRACDIRECTIONALSPREADINGTEST_HPP_
#define DIRACDIRECTIONALSPREADINGTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class DiracDirectionalSpreadingTest : public ::testing::Test {
protected:
    DiracDirectionalSpreadingTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* DIRACDIRECTIONALSPREADINGTEST_HPP_ */
