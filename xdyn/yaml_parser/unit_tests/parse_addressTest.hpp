#ifndef PARSE_ADDRESSTEST_HPP_
#define PARSE_ADDRESSTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class parse_addressTest : public ::testing::Test {
protected:
    parse_addressTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* PARSE_OUTPUTTEST_HPP_ */
