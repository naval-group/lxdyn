/*
 * pairwise_sumTest.hpp
 *
 * \date May 19, 2014
 * \author cec
 */

#ifndef PAIRWISE_SUMTEST_HPP_
#define PAIRWISE_SUMTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class pairwise_sumTest : public ::testing::Test
{
    protected:
        pairwise_sumTest();
        virtual ~pairwise_sumTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* PAIRWISE_SUMTEST_HPP_ */
