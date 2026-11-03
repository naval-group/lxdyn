/*
 * CSVFileReaderTest.cpp
 *
 * \date 8 avr. 2012, 14:10:34
 * \author cec
 */

#include "CSVFileReaderTest.hpp"
#include "ssc/csv_file_reader/CSVFileReader.hpp"
#include <iostream>
#include <fstream>
#include <sys/stat.h>


using namespace ssc::csv_file_reader;

CSVFileReaderTest::CSVFileReaderTest() : a(ssc::random_data_generator::DataGenerator(9487))
{
}

CSVFileReaderTest::~CSVFileReaderTest()
{
}

void CSVFileReaderTest::SetUp()
{
}

void CSVFileReaderTest::TearDown()
{
}

inline void remove_file_if_it_exists (const std::string& name)
{
  struct stat buffer;
  if (stat (name.c_str(), &buffer) == 0) remove (name.c_str());
}

TEST_F(CSVFileReaderTest, example)
{
//! [CSVFileReaderTest example]
    const std::string filename("data_for_CSVFileReaderTest");
    remove_file_if_it_exists(filename);
    std::ofstream file(filename.c_str());
    const std::string contents = "A,B,C\n1,2,3\n4,5,6\n7,8,9";
    file << contents;
    file.close();
    const size_t invalid_nb_of_columns = a.random<size_t>().between(0,10).but_not(3);
    ASSERT_THROW(CSVFileReader(filename.c_str(), invalid_nb_of_columns),CSVFileReaderException);
    CSVFileReader validator(filename.c_str(), 3);
    //remove_file_if_it_exists(filename);
//! [CSVFileReaderTest example]
//! [CSVFileReaderTest expected output]
    std::vector<double> current_line;
    ASSERT_TRUE(validator.has_more_data());
    current_line = validator.get_line();
    ASSERT_EQ(3,current_line.size());
    ASSERT_EQ(1, current_line.at(0));
    ASSERT_EQ(2, current_line.at(1));
    ASSERT_EQ(3, current_line.at(2));
    ASSERT_TRUE(validator.has_more_data());
    current_line = validator.get_line();
    ASSERT_EQ(3,current_line.size());
    ASSERT_EQ(4, current_line.at(0));
    ASSERT_EQ(5, current_line.at(1));
    ASSERT_EQ(6, current_line.at(2));
    ASSERT_TRUE(validator.has_more_data());
    current_line = validator.get_line();
    ASSERT_EQ(3,current_line.size());
    ASSERT_EQ(7, current_line.at(0));
    ASSERT_EQ(8, current_line.at(1));
    ASSERT_EQ(9, current_line.at(2));
    ASSERT_FALSE(validator.has_more_data());
    ASSERT_THROW(validator.get_line(),CSVFileReaderException);
//! [CSVFileReaderTest expected output]
}

TEST_F(CSVFileReaderTest, example_without_opening_file)
{
    const std::string contents = "A,B,C\n1,2,3\n4,5,6\n7,8,9";
    CSVFileReader validator(contents);
    std::vector<double> current_line;
    ASSERT_TRUE(validator.has_more_data());
    current_line = validator.get_line();
    ASSERT_EQ(3,current_line.size());
    ASSERT_EQ(1, current_line.at(0));
    ASSERT_EQ(2, current_line.at(1));
    ASSERT_EQ(3, current_line.at(2));
    ASSERT_TRUE(validator.has_more_data());
    current_line = validator.get_line();
    ASSERT_EQ(3,current_line.size());
    ASSERT_EQ(4, current_line.at(0));
    ASSERT_EQ(5, current_line.at(1));
    ASSERT_EQ(6, current_line.at(2));
    ASSERT_TRUE(validator.has_more_data());
    current_line = validator.get_line();
    ASSERT_EQ(3,current_line.size());
    ASSERT_EQ(7, current_line.at(0));
    ASSERT_EQ(8, current_line.at(1));
    ASSERT_EQ(9, current_line.at(2));
    ASSERT_FALSE(validator.has_more_data());
    ASSERT_THROW(validator.get_line(),CSVFileReaderException);
}

TEST_F(CSVFileReaderTest, can_retrieve_a_map_containing_the_values_by_column)
{
    const std::string contents = "A,B,C,D\n1,2,3,5.46\n4,5,6,8.54\n7,8,9,2.3337";
    const CSVFileReader validator(contents);
    std::map<std::string,std::vector<double> > map = validator.get_map();
    ASSERT_EQ(4, map.size());
    ASSERT_EQ(3, map["A"].size());
    ASSERT_EQ(3, map["B"].size());
    ASSERT_EQ(3, map["C"].size());
    ASSERT_DOUBLE_EQ(1, map["A"][0]);
    ASSERT_DOUBLE_EQ(2, map["B"][0]);
    ASSERT_DOUBLE_EQ(3, map["C"][0]);
    ASSERT_DOUBLE_EQ(5.46, map["D"][0]);
    ASSERT_DOUBLE_EQ(4, map["A"][1]);
    ASSERT_DOUBLE_EQ(5, map["B"][1]);
    ASSERT_DOUBLE_EQ(6, map["C"][1]);
    ASSERT_DOUBLE_EQ(8.54, map["D"][1]);
    ASSERT_DOUBLE_EQ(7, map["A"][2]);
    ASSERT_DOUBLE_EQ(8, map["B"][2]);
    ASSERT_DOUBLE_EQ(9, map["C"][2]);
    ASSERT_DOUBLE_EQ(2.3337, map["D"][2]);
}
