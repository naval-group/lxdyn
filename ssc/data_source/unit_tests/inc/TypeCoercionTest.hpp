/*
 * TypeCoercionTest.hpp
 *
 * \date 17 juin 2013, 14:54:02
 * \author cec
 */

#ifndef TYPECOERCIONTEST_HPP_
#define TYPECOERCIONTEST_HPP_

#include "gtest/gtest.h"

#include "ssc/random_data_generator/DataGenerator.hpp"

class TypeCoercionTest : public ::testing::Test
{
    protected:
        TypeCoercionTest();
        virtual ~TypeCoercionTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* TYPECOERCIONTEST_HPP_ */
