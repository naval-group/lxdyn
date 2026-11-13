/*
 * StatesFilterTest.hpp
 *
 *  Created on: Sep 8, 2021
 *      Author: cady
 */

#ifndef STATESFILTERTEST_HPP_
#define STATESFILTERTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class StatesFilterTest : public ::testing::Test {
protected:
    StatesFilterTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* STATESFILTERTEST_HPP_ */
