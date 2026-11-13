/*
 * KtKqForceModelTest.hpp
 *
 *  Created on: Jun 27, 2015
 *      Author: cady
 */

#ifndef KTKQFORCEMODELTEST_HPP_
#define KTKQFORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class KtKqForceModelTest : public ::testing::Test {
protected:
    KtKqForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* KTKQFORCEMODELTEST_HPP_ */
