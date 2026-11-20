/*
 * ObserverTests.hpp
 *
 *  Created on: May 6, 2015
 *      Author: cady
 */

#ifndef OBSERVERTESTS_HPP_
#define OBSERVERTESTS_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class ObserverTests : public ::testing::Test {
protected:
    ObserverTests();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* OBSERVERTESTS_HPP_ */
