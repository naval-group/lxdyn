/*
 * DiscretizeTest.hpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cady
 */

#ifndef DISCRETIZETEST_HPP_
#define DISCRETIZETEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class discretizeTest : public ::testing::Test {
protected:
    discretizeTest();
    std::vector<double> random_increasing_vector_of_size(const size_t n) const;
    ssc::random_data_generator::DataGenerator a;
};

#endif /* DISCRETIZETEST_HPP_ */
