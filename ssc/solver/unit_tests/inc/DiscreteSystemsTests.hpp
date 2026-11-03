#ifndef DISCRETE_SYSTEMS_TESTS_HPP_
#define DISCRETE_SYSTEMS_TESTS_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class DiscreteSystemsTests : public ::testing::Test
{
    protected:
        DiscreteSystemsTests();
        virtual ~DiscreteSystemsTests();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* DISCRETE_SYSTEMS_TESTS_HPP_ */
