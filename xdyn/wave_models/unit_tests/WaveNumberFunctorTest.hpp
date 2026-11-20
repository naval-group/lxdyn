/*
 * WaveNumberFunctorTest.hpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cady
 */

#ifndef WAVENUMBERFUNCTORTEST_HPP_
#define WAVENUMBERFUNCTORTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class WaveNumberFunctorTest : public ::testing::Test {
protected:
    WaveNumberFunctorTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* WAVENUMBERFUNCTORTEST_HPP_ */
