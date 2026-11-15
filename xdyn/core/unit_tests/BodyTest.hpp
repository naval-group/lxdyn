/*
 * BodyTest.hpp
 *
 *  Created on: Jan 8, 2015
 *      Author: cady
 */

#ifndef BODYTEST_HPP_
#define BODYTEST_HPP_

#include "Body.hpp"
#include <ssc/random_data_generator/DataGenerator.hpp>
#include "gtest/gtest.h"

class BodyTest : public ::testing::Test
{
    protected:
        BodyTest();
        ssc::random_data_generator::DataGenerator a;
        const BodyPtr& body;
};

#endif  /* BODYTEST_HPP_ */
