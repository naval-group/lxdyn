/*
 * BretschneiderSpectrumTest.hpp
 *
 *  Created on: Jul 30, 2014
 *      Author: cady
 */

#ifndef BRETSCHNEIDERSPECTRUMTEST_HPP_
#define BRETSCHNEIDERSPECTRUMTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class BretschneiderSpectrumTest : public ::testing::Test {
protected:
    BretschneiderSpectrumTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* BRETSCHNEIDERSPECTRUMTEST_HPP_ */
