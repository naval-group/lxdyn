/*
 * TextFileReader.hpp
 *
 * \date 21 mars 2012, 15:13:03
 * \author cec
 */

#ifndef TEXTFILEREADER_HPP_
#define TEXTFILEREADER_HPP_

#include <string>
#include <vector>

namespace ssc
{
    namespace text_file_reader
    {
        /** \author cec
         *  \brief This class was created to separate file reading from YAML parsing.
         *  \details It has the following responsibilities:
         *  - Attempt to read all the supplied text files & report any errors
         *  - Can return the contents read as a string
         *  \section ex1 Example
         *  \snippet text_file_reader/unit_tests/src/TextFileReaderTest.cpp TextFileReaderTest example
         *  \section ex2 Expected output
         *  \snippet text_file_reader/unit_tests/src/TextFileReaderTest.cpp TextFileReaderTest expected output
         */

        class TextFileReader
        {
            public:
                TextFileReader(const std::string& filename);
                TextFileReader(const std::vector<std::string>& filenames);
                std::string get_contents() const;

            private:
                TextFileReader();
                void fill(const std::string& filename);
                std::string contents;
        };
    }
}

#endif /* TEXTFILEREADER_HPP_ */
