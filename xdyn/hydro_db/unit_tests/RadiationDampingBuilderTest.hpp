/*
 * DampingMatrixInterpolatorTest.hpp
 *
 *  Created on: Nov 27, 2014
 *      Author: cady
 */

#ifndef DAMPINGMATRIXINTERPOLATORTEST_HPP_
#define DAMPINGMATRIXINTERPOLATORTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class RadiationDampingBuilderTest : public ::testing::Test {
protected:
    RadiationDampingBuilderTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* DAMPINGMATRIXINTERPOLATORTEST_HPP_ */
