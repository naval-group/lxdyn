/*
 * QuadraticDampingForceModelTest.hpp
 *
 *  Created on: Oct 2, 2014
 *      Author: jacquenot
 */

#ifndef QUADRATICDAMPINGFORCEMODELTEST_HPP_
#define QUADRATICDAMPINGFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class QuadraticDampingForceModelTest : public ::testing::Test
{
    protected:
    QuadraticDampingForceModelTest();
        ~QuadraticDampingForceModelTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* QUADRATICDAMPINGFORCEMODELTEST_HPP_ */
