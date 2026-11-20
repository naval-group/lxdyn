#ifndef OBSERVERS_AND_API_UNIT_TESTS_INC_CONFBUILDERTEST_HPP_
#define OBSERVERS_AND_API_UNIT_TESTS_INC_CONFBUILDERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class ConfBuilderTest : public ::testing::Test {
protected:
    ConfBuilderTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* OBSERVERS_AND_API_UNIT_TESTS_INC_CONFBUILDERTEST_HPP_ */
