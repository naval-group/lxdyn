/*
 * MMGPropellerForceModelTest.hpp
 *
 *  Created on: Oct 3, 2024
 *      Author: fbickert
 */

#ifndef MMGPROPELLERFORCEMODELTEST_HPP_
#define MMGPROPELLERFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class MMGPropellerForceModelTest : public ::testing::Test
{
    protected:
        MMGPropellerForceModelTest();
        virtual ~MMGPropellerForceModelTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* MMGPROPELLERFORCEMODELTEST_HPP_ */
