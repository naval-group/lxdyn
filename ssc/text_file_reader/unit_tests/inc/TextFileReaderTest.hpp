/*
 * TextFileReaderTest.hpp
 *
 * \date 21 mars 2012, 15:18:20
 * \author cec
 */

#ifndef TEXTFILEREADERTEST_HPP_
#define TEXTFILEREADERTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"


class TextFileReaderTest : public ::testing::Test
{
    protected:
        TextFileReaderTest();
        virtual ~TextFileReaderTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* TEXTFILEREADERTEST_HPP_ */
