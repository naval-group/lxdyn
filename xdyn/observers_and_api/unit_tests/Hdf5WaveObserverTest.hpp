#ifndef HDF5WAVEOBSERVERTEST_HPP_
#define HDF5WAVEOBSERVERTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class SimHdf5WaveObserverTest : public ::testing::Test
{
    protected:
        SimHdf5WaveObserverTest();
        ssc::random_data_generator::DataGenerator a;
};

#endif
