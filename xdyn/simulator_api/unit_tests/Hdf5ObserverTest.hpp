#ifndef HDF5OBSERVERTEST_HPP_
#define HDF5OBSERVERTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class Hdf5ObserverTest : public ::testing::Test {
protected:
    Hdf5ObserverTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif
