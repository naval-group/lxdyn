#ifndef HDF5OBSERVERTEST_HPP_
#define HDF5OBSERVERTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class Hdf5ObserverTest : public ::testing::Test
{
    protected:
        Hdf5ObserverTest();
        ~Hdf5ObserverTest() override{}
        void SetUp() override{}
        void TearDown() override{}
        ssc::random_data_generator::DataGenerator a;
};

#endif
