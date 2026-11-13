/*
 * Cos2sDirectionalSpreadingTest.hpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cady
 */

#ifndef COS2SDIRECTIONALSPREADINGTEST_HPP_
#define COS2SDIRECTIONALSPREADINGTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class Cos2sDirectionalSpreadingTest : public ::testing::Test {
protected:
    Cos2sDirectionalSpreadingTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* COS2SDIRECTIONALSPREADINGTEST_HPP_ */
