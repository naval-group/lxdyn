#ifndef SCHEDULER_TESTS_HPP_
#define SCHEDULER_TESTS_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class SchedulerTests : public ::testing::Test
{
    protected:
        SchedulerTests();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* SCHEDULER_TESTS_HPP_ */
