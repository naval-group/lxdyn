/*
 * HDBParserTest.hpp
 *
 *  Created on: Nov 13, 2014
 *      Author: cady
 */

#ifndef HDBPARSERTEST_HPP_
#define HDBPARSERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class HDBParserTest : public ::testing::Test {
protected:
    HDBParserTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* HDBPARSERTEST_HPP_ */
