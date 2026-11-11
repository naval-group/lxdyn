/*
 * FilonTest.hpp
 *
 * \date Dec 10, 2014
 * \author cec
 */


#ifndef FILONTEST_HPP_
#define FILONTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class FilonTest : public ::testing::Test
{
    protected:
        FilonTest();
        ~FilonTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* FILONTEST_HPP_ */
