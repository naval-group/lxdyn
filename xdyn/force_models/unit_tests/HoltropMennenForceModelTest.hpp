#ifndef FORCE_MODELS_UNIT_TESTS_INC_HOLTROPMENNENFORCEMODELTEST_HPP_
#define FORCE_MODELS_UNIT_TESTS_INC_HOLTROPMENNENFORCEMODELTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator/DataGenerator.hpp>
#include "HoltropMennenForceModel.hpp"

class HoltropMennenForceModelTest : public ::testing::Test
{
protected:
    HoltropMennenForceModelTest();
    ~HoltropMennenForceModelTest() override;
    void SetUp() override;
    void TearDown() override;
    ssc::random_data_generator::DataGenerator a;
    EnvironmentAndFrames env;
};

#endif /* FORCE_MODELS_UNIT_TESTS_INC_HOLTROPMENNENFORCEMODELTEST_HPP_ */
