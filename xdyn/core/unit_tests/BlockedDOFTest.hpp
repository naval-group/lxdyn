/*
 * BlockedDOFTest.hpp
 *
 *  Created on: Aug 19, 2015
 *      Author: cady
 */

#ifndef BLOCKEDDOFTEST_HPP_
#define BLOCKEDDOFTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class BlockedDOFTest : public ::testing::Test {
protected:
    BlockedDOFTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* BLOCKEDDOFTEST_HPP_ */
