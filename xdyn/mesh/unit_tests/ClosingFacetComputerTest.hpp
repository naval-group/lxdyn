/*
 * ClosingFacetComputerTest.hpp
 *
 *  Created on: Feb 27, 2015
 *      Author: cady
 */


#ifndef CLOSINGFACETCOMPUTERTEST_HPP_
#define CLOSINGFACETCOMPUTERTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class ClosingFacetComputerTest : public ::testing::Test
{
    protected:
        ClosingFacetComputerTest();
        ~ClosingFacetComputerTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* CLOSINGFACETCOMPUTERTEST_HPP_ */
