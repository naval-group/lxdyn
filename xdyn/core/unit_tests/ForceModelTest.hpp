/*
 * ForceModelTest.hpp
 *
 *  Created on: May 11, 2015
 *      Author: cady
 */

#ifndef FORCEMODELTEST_HPP_
#define FORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class ForceModelTest : public ::testing::Test {
protected:
    ForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* FORCEMODELTEST_HPP_ */
