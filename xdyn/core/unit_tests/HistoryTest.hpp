/*
 * HistoryTest.hpp
 *
 *  Created on: Nov 26, 2014
 *      Author: cady
 */

#ifndef HISTORYTEST_HPP_
#define HISTORYTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class HistoryTest : public ::testing::Test {
protected:
    HistoryTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* HISTORYTEST_HPP_ */
