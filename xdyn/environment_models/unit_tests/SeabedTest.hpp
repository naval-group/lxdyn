/*
 * SeabedTest.hpp
 */

#ifndef SEABEDTEST_HPP_
#define SEABEDTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "Seabed.hpp"
#include "gtest/gtest.h"

class SeabedTest : public ::testing::Test {
protected:
    SeabedTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* SEABEDTEST_HPP_ */
