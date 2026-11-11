#ifndef ROTATION_MATRIX_BUILDERS_TEST_HPP_
#define ROTATION_MATRIX_BUILDERS_TEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class rotation_matrix_builders_Test : public ::testing::Test
{
    protected:
        rotation_matrix_builders_Test();
        ~rotation_matrix_builders_Test() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* ROTATION_MATRIX_BUILDERS_TEST_HPP_ */
