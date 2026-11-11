#ifndef HDF5WAVEOBSERVERTEST_HPP_
#define HDF5WAVEOBSERVERTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class SimHdf5WaveObserverTest : public ::testing::Test
{
    protected:
        SimHdf5WaveObserverTest();
        ~SimHdf5WaveObserverTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif
