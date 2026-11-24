#ifndef JSONOBSERVERTEST_HPP_
#define JSONOBSERVERTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class JsonObserverTest : public ::testing::Test {
protected:
    JsonObserverTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif
