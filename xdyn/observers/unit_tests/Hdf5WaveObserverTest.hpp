#ifndef HDF5WAVEOBSERVERTEST_HPP_
#define HDF5WAVEOBSERVERTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class SimHdf5WaveObserverTest : public ::testing::Test {
protected:
    SimHdf5WaveObserverTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif
