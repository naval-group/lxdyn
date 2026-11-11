#ifndef DISCRETE_SYSTEMS_TESTS_HPP_
#define DISCRETE_SYSTEMS_TESTS_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class DiscreteSystemsTests : public ::testing::Test
{
    protected:
        DiscreteSystemsTests();
        ~DiscreteSystemsTests() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* DISCRETE_SYSTEMS_TESTS_HPP_ */
