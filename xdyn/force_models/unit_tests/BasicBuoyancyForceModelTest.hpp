/*
 * BasicBuoyancyForceModelTest.hpp
 */

#ifndef BASICBUOYANCYFORCEMODELTEST_HPP_
#define BASICBUOYANCYFORCEMODELTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class BasicBuoyancyForceModelTest : public ::testing::Test {
protected:
    BasicBuoyancyForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* BASICBUOYANCYFORCEMODELTEST_HPP_ */
