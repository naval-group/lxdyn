/*
 * LinearHydrostaticForceModelTest.hpp
 *
 *  Created on: Aug 12, 2015
 *      Author: cady
 */


#ifndef LINEARHYDROSTATICFORCEMODELTEST_HPP_
#define LINEARHYDROSTATICFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class LinearHydrostaticForceModelTest : public ::testing::Test
{
    protected:
        LinearHydrostaticForceModelTest();
        ~LinearHydrostaticForceModelTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* LINEARHYDROSTATICFORCEMODELTEST_HPP_ */
