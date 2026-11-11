/*
 * RudderForceModelTest.hpp
 *
 *  Created on: Feb 2, 2015
 *      Author: cady
 */


#ifndef RUDDERFORCEMODELTEST_HPP_
#define RUDDERFORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>
#include <ssc/macros.hpp>
#include "gtest/gtest.h"
#include "xdyn/core/EnvironmentAndFrames.hpp"

class RudderForceModelTest : public ::testing::Test
{
    protected:
        RudderForceModelTest();
        ~RudderForceModelTest() override;
        void SetUp() override;
        void TearDown() override;
        EnvironmentAndFrames get_env();
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* RUDDERFORCEMODELTEST_HPP_ */
