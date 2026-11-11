/*
 * DiracSpectralDensityTest.hpp
 *
 *  Created on: Jul 31, 2014
 *      Author: cady
 */

#ifndef DIRACSPECTRALDENSITYTEST_HPP_
#define DIRACSPECTRALDENSITYTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class DiracSpectralDensityTest : public ::testing::Test
{
    protected:
        DiracSpectralDensityTest();
        ~DiracSpectralDensityTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* DIRACSPECTRALDENSITYTEST_HPP_ */
