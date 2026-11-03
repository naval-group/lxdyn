/*
 * DataSourceTest.hpp
 *
 * \date 13 mars 2013, 15:19:32
 * \author cec
 */

#ifndef DATASOURCETEST_HPP_
#define DATASOURCETEST_HPP_

#include "gtest/gtest.h"

#include "ssc/random_data_generator/DataGenerator.hpp"

class DataSourceTest : public ::testing::Test
{
    protected:
        DataSourceTest();
        virtual ~DataSourceTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;
};



#endif  /* DATASOURCETEST_HPP_ */
