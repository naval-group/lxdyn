/*
 * IndexFinderTest.hpp
 *
 * \date 10 sept. 2013, 10:15:12
 * \author cec
 */

#ifndef INDEXFINDERTEST_HPP_
#define INDEXFINDERTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class IndexFinderTest : public ::testing::Test
{
    protected:
        IndexFinderTest();
        virtual ~IndexFinderTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* INDEXFINDERTEST_HPP_ */
