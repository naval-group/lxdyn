

#ifndef OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORCSTEST_HPP_
#define OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORCSTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class XdynForCSTest : public ::testing::Test
{
    protected:
        XdynForCSTest();
        ~XdynForCSTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* OBSERVERS_AND_API_UNIT_TESTS_INC_XDYNFORCSTEST_HPP_ */
