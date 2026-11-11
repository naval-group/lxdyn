/*
 * GZCurveTest.hpp
 *
 *  Created on: Feb 20, 2015
 *      Author: cady
 */


#ifndef GZCURVETEST_HPP_
#define GZCURVETEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

#include "xdyn/core/Sim.hpp"

class GZCurveTest : public ::testing::Test
{
    protected:
        GZCurveTest();
        ~GZCurveTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
        Sim& sim;
};

#endif  /* GZCURVETEST_HPP_ */
