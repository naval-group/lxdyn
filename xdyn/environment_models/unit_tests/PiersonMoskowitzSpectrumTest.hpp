/*
 * PiersonMoskowitzSpectrumTest.hpp
 *
 *  Created on: Jul 30, 2014
 *      Author: cady
 */

#ifndef PIERSONMOSKOWITZSPECTRUMTEST_HPP_
#define PIERSONMOSKOWITZSPECTRUMTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class PiersonMoskowitzSpectrumTest : public ::testing::Test
{
    protected:
        PiersonMoskowitzSpectrumTest();
        ~PiersonMoskowitzSpectrumTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* PIERSONMOSKOWITZSPECTRUMTEST_HPP_ */
