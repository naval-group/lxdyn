/*
 * coriolis_and_centripetalTest.hpp
 *
 * \date Jun 3, 2014
 * \author cec
 */

#ifndef CORIOLIS_AND_CENTRIPETALTEST_HPP_
#define CORIOLIS_AND_CENTRIPETALTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class coriolis_and_centripetalTest : public ::testing::Test
{
    protected:
        coriolis_and_centripetalTest();
        virtual ~coriolis_and_centripetalTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* CORIOLIS_AND_CENTRIPETALTEST_HPP_ */
