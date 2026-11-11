/*
 * ManeuveringForceModelTest.hpp
 *
 *  Created on: Jan 19, 2015
 *      Author: cady
 */


#ifndef MANEUVERINGFORCEMODELTEST_HPP_
#define MANEUVERINGFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class ManeuveringForceModelTest : public ::testing::Test
{
    protected:
        ManeuveringForceModelTest();
        ~ManeuveringForceModelTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* MANEUVERINGFORCEMODELTEST_HPP_ */
