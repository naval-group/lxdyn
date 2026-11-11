/*
 * CSVFileReaderTest.hpp
 *
 * \date 8 avr. 2012, 14:10:21
 * \author cec
 */

#ifndef VALIDATEAGAINSTCSVTEST_HPP_
#define VALIDATEAGAINSTCSVTEST_HPP_

#include "gtest/gtest.h"
#include "ssc/random_data_generator/DataGenerator.hpp"

class CSVFileReaderTest : public ::testing::Test
{
    protected:
        CSVFileReaderTest();
        ~CSVFileReaderTest() override;
        void SetUp() override;
        void TearDown() override;
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* VALIDATEAGAINSTCSVTEST_HPP_ */
