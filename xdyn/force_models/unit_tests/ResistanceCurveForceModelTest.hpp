/*
 * ResistanceCurveForceModelTest.hpp
 *
 *  Created on: Oct 24, 2014
 *      Author: cady
 */

#ifndef RESISTANCECURVEFORCEMODELTEST_HPP_
#define RESISTANCECURVEFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>

class ResistanceCurveForceModelTest : public ::testing::Test
{
    protected:
        ResistanceCurveForceModelTest();
        ~ResistanceCurveForceModelTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;
};

#endif  /* RESISTANCECURVEFORCEMODELTEST_HPP_ */
