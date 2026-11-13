/*
 * maneuvering_compilerTest.hpp
 *
 *  Created on: Jan 22, 2015
 *      Author: cady
 */

#ifndef MANEUVERING_COMPILERTEST_HPP_
#define MANEUVERING_COMPILERTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class maneuvering_parserTest : public ::testing::Test {
protected:
    maneuvering_parserTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* MANEUVERING_COMPILERTEST_HPP_ */
