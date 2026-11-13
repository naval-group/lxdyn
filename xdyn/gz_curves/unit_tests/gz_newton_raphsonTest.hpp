/*
 * gz_newton_raphsonTest.hpp
 *
 *  Created on: Feb 20, 2015
 *      Author: cady
 */

#ifndef GZ_NEWTON_RAPHSONTEST_HPP_
#define GZ_NEWTON_RAPHSONTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class gz_newton_raphsonTest : public ::testing::Test {
protected:
    gz_newton_raphsonTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* GZ_NEWTON_RAPHSONTEST_HPP_ */
