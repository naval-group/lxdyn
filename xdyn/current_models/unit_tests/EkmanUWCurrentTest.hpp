/*
 * EkmanUWCurrentTest.hpp
 */

#ifndef EKMANUWCURRENTTEST_HPP_
#define EKMANUWCURRENTTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class EkmanUWCurrentTest : public ::testing::Test {
protected:
    EkmanUWCurrentTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* EKMANUWCURRENTTEST_HPP_ */
