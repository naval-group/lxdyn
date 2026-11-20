/*
 * StretchingTest.hpp
 *
 *  Created on: Dec 20, 2016
 *      Author: cady
 */

#ifndef STRETCHINGTEST_HPP_
#define STRETCHINGTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class StretchingTest : public ::testing::Test {
protected:
    StretchingTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* STRETCHINGTEST_HPP_ */
