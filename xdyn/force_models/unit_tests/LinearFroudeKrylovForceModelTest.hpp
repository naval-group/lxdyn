/*
 * LinearFroudeKrylovForceModelTest.hpp
 *
 *  Created on: July 16, 2021
 *      Author: cady
 */


#ifndef LINEARFROUDEKRYLOVFORCEMODELTEST_HPP_
#define LINEARFROUDEKRYLOVFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class LinearFroudeKrylovForceModelTest : public ::testing::Test
{
    protected:
        LinearFroudeKrylovForceModelTest();
        ~LinearFroudeKrylovForceModelTest() override;
        void SetUp() override;
        void TearDown() override;
        double small_relative_error(const double val) const;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* LINEARFROUDEKRYLOVFORCEMODELTEST_HPP_ */
