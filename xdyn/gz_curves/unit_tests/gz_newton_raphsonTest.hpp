/*
 * gz_newton_raphsonTest.hpp
 *
 *  Created on: Feb 20, 2015
 *      Author: cady
 */


#ifndef GZ_NEWTON_RAPHSONTEST_HPP_
#define GZ_NEWTON_RAPHSONTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class gz_newton_raphsonTest : public ::testing::Test
{
    protected:
        gz_newton_raphsonTest();
        ~gz_newton_raphsonTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* GZ_NEWTON_RAPHSONTEST_HPP_ */
