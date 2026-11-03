/*
 * almost_equalTest.hpp
 *
 * \date 26 nov. 2013, 08:41:26
 * \author cec
 */

#ifndef ALMOST_EQUALTEST_HPP_
#define ALMOST_EQUALTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class almost_equalTest : public ::testing::Test
{
    protected:
        almost_equalTest();
        virtual ~almost_equalTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;
        double one_ULP_less(const double x) const;
        double one_ULP_more(const double x) const;
        double five_ULP_less(double x) const;
        double five_ULP_more(double x) const;

};

#endif  /* ALMOST_EQUALTEST_HPP_ */
