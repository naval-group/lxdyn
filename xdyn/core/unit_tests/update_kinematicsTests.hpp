/*
 * update_kinematicsTests.hpp
 *
 *  Created on: Jun 19, 2014
 *      Author: cady
 */

#ifndef UPDATE_KINEMATICSTESTS_HPP_
#define UPDATE_KINEMATICSTESTS_HPP_

#include <ssc/random_data_generator.hpp>

#include "gtest/gtest.h"

class update_kinematicsTests : public ::testing::Test {
protected:
    update_kinematicsTests();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* UPDATE_KINEMATICSTESTS_HPP_ */
