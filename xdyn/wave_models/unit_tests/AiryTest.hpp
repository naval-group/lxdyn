/*
 * AiryTest.hpp
 *
 *  Created on: Aug 1, 2014
 *      Author: cady
 */

#ifndef AIRYTEST_HPP_
#define AIRYTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class AiryTest : public ::testing::Test {
protected:
    AiryTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* AIRYTEST_HPP_ */
