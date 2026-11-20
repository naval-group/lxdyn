

#ifndef OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORCSTEST_HPP_
#define OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORCSTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class XdynForCSTest : public ::testing::Test {
protected:
    XdynForCSTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORCSTEST_HPP_ */
