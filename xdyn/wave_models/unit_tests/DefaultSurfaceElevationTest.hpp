#ifndef DEFAULTSURFACELEVATIONTEST_HPP_
#define DEFAULTSURFACELEVATIONTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class DefaultSurfaceElevationTest : public ::testing::Test {
protected:
    DefaultSurfaceElevationTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* DEFAULTSURFACELEVATIONTEST_HPP_ */
