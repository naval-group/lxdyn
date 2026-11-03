#ifndef SCHEDULER_TESTS_HPP_
#define SCHEDULER_TESTS_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class SchedulerTests : public ::testing::Test
{
    protected:
        SchedulerTests();
        virtual ~SchedulerTests();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* SCHEDULER_TESTS_HPP_ */
