/*
 * ManeuveringDataSourceBuilderTest.hpp
 *
 *  Created on: Jan 27, 2015
 *      Author: cady
 */

#ifndef MANEUVERINGDATASOURCEBUILDERTEST_HPP_
#define MANEUVERINGDATASOURCEBUILDERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class maneuvering_DataSource_builderTest : public ::testing::Test {
protected:
    maneuvering_DataSource_builderTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* MANEUVERINGDATASOURCEBUILDERTEST_HPP_ */
