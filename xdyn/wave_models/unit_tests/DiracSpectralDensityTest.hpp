/*
 * DiracSpectralDensityTest.hpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cady
 */

#ifndef DIRACSPECTRALDENSITYTEST_HPP_
#define DIRACSPECTRALDENSITYTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class DiracSpectralDensityTest : public ::testing::Test {
protected:
    DiracSpectralDensityTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* DIRACSPECTRALDENSITYTEST_HPP_ */
