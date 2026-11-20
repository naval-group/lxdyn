/*
 * WaveSpectralDensityTest.hpp
 *
 *  Created on: Jun 12, 2015
 *      Author: cady
 */

#ifndef WAVESPECTRALDENSITYTEST_HPP_
#define WAVESPECTRALDENSITYTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class WaveSpectralDensityTest : public ::testing::Test {
protected:
    WaveSpectralDensityTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* WAVESPECTRALDENSITYTEST_HPP_ */
