/*
 * QuadraticDampingForceModelTest.hpp
 *
 *  Created on: Oct 2, 2014
 *      Author: jacquenot
 */

#ifndef QUADRATICDAMPINGFORCEMODELTEST_HPP_
#define QUADRATICDAMPINGFORCEMODELTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class QuadraticDampingForceModelTest : public ::testing::Test {
protected:
    QuadraticDampingForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* QUADRATICDAMPINGFORCEMODELTEST_HPP_ */
