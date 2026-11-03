#ifndef RANDOM_DATA_GENERATOR_TESTS_HPP
#define RANDOM_DATA_GENERATOR_TESTS_HPP

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class DataGeneratorTests : public ::testing::Test
{
    protected:
        DataGeneratorTests() : a(ssc::random_data_generator::DataGenerator(95442))
                                {}
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};



#endif
