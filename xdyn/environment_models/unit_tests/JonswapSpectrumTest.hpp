/*
 * JonswapSpectrumTest.hpp
 *
 *  Created on: Jul 30, 2014
 *      Author: cady
 */

#ifndef JONSWAPSPECTRUMTEST_HPP_
#define JONSWAPSPECTRUMTEST_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class JonswapSpectrumTest : public ::testing::Test {
protected:
    JonswapSpectrumTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* JONSWAPSPECTRUMTEST_HPP_ */
