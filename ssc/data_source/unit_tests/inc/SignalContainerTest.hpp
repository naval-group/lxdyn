/*
 * SignalContainerTest.hpp
 *
 * \date 18 mars 2013, 17:59:57
 * \author cec
 */

#ifndef SIGNALCONTAINERTEST_HPP_
#define SIGNALCONTAINERTEST_HPP_

#include "gtest/gtest.h"

#include "ssc/random_data_generator/DataGenerator.hpp"

class SignalContainerTest : public ::testing::Test
{
    protected:
        SignalContainerTest();
        virtual ~SignalContainerTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* SIGNALCONTAINERTEST_HPP_ */
