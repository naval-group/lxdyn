/*
 * BasicBuoyancyForceModelTest.hpp
 */

#ifndef BASICBUOYANCYFORCEMODELTEST_HPP_
#define BASICBUOYANCYFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class BasicBuoyancyForceModelTest : public ::testing::Test
{
    protected:
        BasicBuoyancyForceModelTest();
        ~BasicBuoyancyForceModelTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* BASICBUOYANCYFORCEMODELTEST_HPP_ */
