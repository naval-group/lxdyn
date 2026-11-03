#ifndef CSVFILEREADER_HPP_
#define CSVFILEREADER_HPP_

#include "ssc/exception_handling/Exception.hpp"
#include <vector>
#include <string>
#include <map>

namespace ssc
{
    namespace csv_file_reader
    {
        class CSVFileReaderException: public ::ssc::exception_handling::Exception {
        public:
            CSVFileReaderException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                Exception(message, file, function, line) {
            }
        };

        class CSVFileReader
        {
            public:
                CSVFileReader(const char* filename, const size_t expected_nb_of_columns, const char separator=',');
                CSVFileReader(const std::string& contents, const char separator=',');
                bool has_more_data() const;
                std::vector<double> get_line();
                std::map<std::string,std::vector<double> > get_map() const;

            private:
                CSVFileReader();
                std::vector<std::string> titles;
                std::vector<std::vector<double> > values;
                const std::vector<std::string> convert_line_to_vector_of_strings(const std::string& line, const char separator) const;
                const std::vector<double> convert_line_to_vector_of_doubles(const std::string& line, const char separator) const;
                void extract_column_titles(std::istream& file, const char separator);
                void extract_values(std::istream& file, const char separator);
                std::vector<std::vector<double> >::const_iterator that_line;
                std::map<std::string,std::vector<double> > map;
        };
    }
}

#endif

