/*
 * ObserverTests.hpp
 *
 *  Created on: May 6, 2015
 *      Author: cady
 */


#ifndef OBSERVERTESTS_HPP_
#define OBSERVERTESTS_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class ObserverTests : public ::testing::Test
{
    protected:
        ObserverTests();
        ~ObserverTests() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* OBSERVERTESTS_HPP_ */
