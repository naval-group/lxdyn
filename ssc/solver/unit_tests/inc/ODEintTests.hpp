#ifndef ODEINTTESTS_HPP_
#define ODEINTTESTS_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class ODEintTests : public ::testing::Test
{
    protected:
        ODEintTests();
        ~ODEintTests() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* ODEINTTESTS_HPP_ */
