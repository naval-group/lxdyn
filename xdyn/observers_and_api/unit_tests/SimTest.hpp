/*
 * SimTest.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#ifndef SIMTEST_HPP_
#define SIMTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>
#include "xdyn/external_data_structures/GeometricTypes3d.hpp"

class SimTest : public ::testing::Test
{
    protected:
        SimTest();
        ~SimTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
        const VectorOfVectorOfPoints& test_ship_stl;
};

#endif  /* SIMTEST_HPP_ */
