/*
 * TextFileReaderTest.cpp
 *
 * \date 21 mars 2012, 15:18:47
 * \author cec
 */

#include "TextFileReaderTest.hpp"
#include "ssc/text_file_reader/TextFileReader.hpp"
#include "ssc/text_file_reader/TextFileReaderException.hpp"
#include "ssc/macros/test_macros.hpp"
#include <iostream>
#include <fstream>

using namespace ssc::text_file_reader;

TextFileReaderTest::TextFileReaderTest() : a(ssc::random_data_generator::DataGenerator(718293))
{
}

TextFileReaderTest::~TextFileReaderTest()
{

}

void TextFileReaderTest::SetUp()
{

}

void TextFileReaderTest::TearDown()
{
}

inline void remove_file_if_it_exists (const std::string& name)
{
  struct stat buffer;
  if (stat (name.c_str(), &buffer) == 0) remove (name.c_str());
}

void create_file(const std::string& filename, const std::string& contents);
void create_file(const std::string& filename, const std::string& contents)
{
    remove_file_if_it_exists(filename);
    std::ofstream file(filename);
    file << contents;
    file.close();
}


TEST_F(TextFileReaderTest, example)
{
//! [TextFileReaderTest example]
    const std::string filename1 = "data_for_TextFileReaderTest_1";
    const std::string contents_file1 = a.random<std::string>();
    create_file(filename1, contents_file1);
    const std::string filename2 = "data_for_TextFileReaderTest_2";
    const std::string contents_file2 = a.random<std::string>();
    create_file(filename2, contents_file2);
    std::vector<std::string> filenames;
    filenames.push_back(filename1);
    filenames.push_back(filename2);
    TextFileReader reader(filenames);
    remove_file_if_it_exists(filename1);
    remove_file_if_it_exists(filename2);
//! [TextFileReaderTest example]
//! [TextFileReaderTest expected output]
    const std::string newline = "\n";
    ASSERT_EQ(contents_file1+newline+newline+contents_file2, reader.get_contents());
//! [TextFileReaderTest expected output]
}

TEST_F(TextFileReaderTest, should_represent_newlines_correctly)
{
    const std::string filename = "data_for_TextFileReaderTest";
    const std::string contents_file = a.random<std::string>()()
                                             + std::string("\n")
                                     + a.random<std::string>()()
                                     + std::string("\n")
                                     + a.random<std::string>()();
    create_file(filename, contents_file);
    TextFileReader reader(std::vector<std::string>(1, filename));
    remove_file_if_it_exists(filename);
    ASSERT_EQ(contents_file, reader.get_contents());
}

TEST_F(TextFileReaderTest, should_issue_a_warning_if_a_file_is_empty)
{
    std::stringstream error_stream;
    // Redirect cerr to our stringstream buffer or any other ostream
    std::streambuf* orig = std::cerr.rdbuf(error_stream.rdbuf());
    ASSERT_TRUE(error_stream.str().empty());
    // Call TextFileReader
    const std::string filename = "data_for_TextFileReaderTest";
    create_file(filename, "");
    TextFileReader(std::vector<std::string>(1, filename));
    remove_file_if_it_exists(filename);
    const std::string error = error_stream.str();
    // Restore cerr's buffer
    std::cerr.rdbuf(orig);
    ASSERT_FALSE(error.empty());
}

TEST_F(TextFileReaderTest, should_throw_if_file_does_not_exist)
{
    const std::string filename = "data_for_TextFileReaderTest";
    remove_file_if_it_exists(filename);
    ASSERT_THROW(TextFileReader(std::vector<std::string>(1, filename)), TextFileReaderException);
}
