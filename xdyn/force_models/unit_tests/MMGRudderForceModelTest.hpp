/*
 * MMGRudderForceModelTest.hpp
 *
 *  Created on: Oct 11, 2024
 *      Author: fbickert
 */

#ifndef MMGRUDDERFORCEMODELTEST_HPP_
#define MMGRUDDERFORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>
#include <ssc/macros.hpp>
#include "gtest/gtest.h"
#include "xdyn/core/EnvironmentAndFrames.hpp"

class MMGRudderForceModelTest : public ::testing::Test
{
    protected:
        MMGRudderForceModelTest();
        virtual ~MMGRudderForceModelTest();
        virtual void SetUp();
        virtual void TearDown();
        EnvironmentAndFrames get_env();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* MMGRUDDERFORCEMODELTEST_HPP_ */
