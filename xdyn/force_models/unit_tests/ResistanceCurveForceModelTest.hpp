/*
 * ResistanceCurveForceModelTest.hpp
 *
 *  Created on: Oct 24, 2014
 *      Author: cady
 */

#ifndef RESISTANCECURVEFORCEMODELTEST_HPP_
#define RESISTANCECURVEFORCEMODELTEST_HPP_

#include <ssc/random_data_generator/DataGenerator.hpp>

#include "gtest/gtest.h"

class ResistanceCurveForceModelTest : public ::testing::Test {
protected:
    ResistanceCurveForceModelTest();
    ssc::random_data_generator::DataGenerator a;
};

#endif /* RESISTANCECURVEFORCEMODELTEST_HPP_ */
