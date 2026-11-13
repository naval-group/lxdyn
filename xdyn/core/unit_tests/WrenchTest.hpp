#ifndef CORE_UNIT_TESTS_INC_WRENCHTEST_HPP_
#define CORE_UNIT_TESTS_INC_WRENCHTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"
#include "xdyn/core/Wrench.hpp"

class WrenchTest : public ::testing::Test {
public:
    WrenchTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* CORE_UNIT_TESTS_INC_WRENCHTEST_HPP_ */
