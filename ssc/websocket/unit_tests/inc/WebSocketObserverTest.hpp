#ifndef WEBSOCKETOBSERVERTEST_HPP_
#define WEBSOCKETOBSERVERTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class WebSocketObserverTest : public ::testing::Test
{
    protected:
        WebSocketObserverTest() : a(ssc::random_data_generator::DataGenerator(42272)){}
        ~WebSocketObserverTest() override{}
        void SetUp() override{}
        void TearDown() override{}
        ssc::random_data_generator::DataGenerator a;
};

#endif
