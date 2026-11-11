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
        ~SignalContainerTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* SIGNALCONTAINERTEST_HPP_ */
