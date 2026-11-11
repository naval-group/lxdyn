/*
 * CSVFileReaderTest.hpp
 *
 *  Created on: March 29, 2022
 *      Author: cady
 */

#ifndef CSVFILEREADERTEST_HPP_
#define CSVFILEREADERTEST_HPP_

#include "gtest/gtest.h"
#include "CSVLineByLineReader.hpp"
#include "TempFile.hpp"
#include <ssc/random_data_generator.hpp>

class CSVLineByLineReaderTest : public ::testing::Test
{
    protected:
        CSVLineByLineReaderTest();
        ~CSVLineByLineReaderTest() override;
        void SetUp() override;
        void TearDown() override;
        CSVLineByLineReader get_reader() const;
        TempFile csv;
        ssc::random_data_generator::DataGenerator a;
};


#endif /* CSVFILEREADERTEST_HPP_ */
