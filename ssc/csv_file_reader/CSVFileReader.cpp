#include "ssc/csv_file_reader/CSVFileReader.hpp"
#include "ssc/macros/tr1_macros.hpp"

#include <cstdlib> // For atof
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>

using namespace ssc::csv_file_reader;

CSVFileReader::CSVFileReader(const char* filename, const size_t expected_nb_of_columns, const char separator) : titles(std::vector<std::string>()),
        values(std::vector<std::vector<double> >()),
        that_line(values.begin()),
        map(std::map<std::string,std::vector<double> >())
{
    std::ifstream file(filename);
    if (not (file.good()))
    {
        std::stringstream ss;
        ss << "Unable to open CSV file: " << std::string(filename) << std::endl;
        THROW(__PRETTY_FUNCTION__,CSVFileReaderException, ss.str());
    }

    extract_column_titles(file, separator);
    if (titles.size() != expected_nb_of_columns)
    {
        std::stringstream ss;
        ss << "Invalid number of columns. Expected " << expected_nb_of_columns << ", got " << titles.size() << std::endl;
        THROW(__PRETTY_FUNCTION__, CSVFileReaderException, ss.str());
    }
    extract_values(file, separator);
    that_line = values.begin();
}

CSVFileReader::CSVFileReader(const std::string& contents, const char separator) : titles(std::vector<std::string>()),
        values(std::vector<std::vector<double> >()),
        that_line(values.begin()),
        map(std::map<std::string,std::vector<double> >())
{
    std::istringstream ss(contents);
    extract_column_titles(ss, separator);
    extract_values(ss, separator);
    that_line = values.begin();
}

const std::vector<std::string> CSVFileReader::convert_line_to_vector_of_strings(const std::string& line, const char separator) const
{
    std::istringstream line_stream(line);
    std::string field;
    bool end_of_file_was_reached = true;
    std::vector<std::string> values;
    while (std::getline(line_stream, field, separator))
    {
        values.push_back(field);
        end_of_file_was_reached = line_stream.eof();
    }
    if (not(end_of_file_was_reached)) values.push_back(std::string());
    return values;
}

const std::vector<double> CSVFileReader::convert_line_to_vector_of_doubles(const std::string& line, const char separator) const
{
    const std::vector<std::string> values = convert_line_to_vector_of_strings(line, separator);
    std::vector<double> ret;
    std::vector<std::string>::const_iterator it = values.begin();
    for (;it!=values.end() ; ++it) ret.push_back(atof(it->c_str()));
    return ret;
}

void CSVFileReader::extract_column_titles(std::istream& file, const char separator)
{
    std::string current_line;
    getline(file, current_line, '\n');
    titles = convert_line_to_vector_of_strings(current_line, separator);
    std::vector<std::string>::const_iterator it = titles.begin();
    for (;it != titles.end() ; ++it)
    {
        map[*it] = std::vector<double>();
    }
}

void CSVFileReader::extract_values(std::istream& file, const char separator)
{
    std::string current_line;
    while (getline(file, current_line, '\n'))
    {
        std::string nospace = current_line;
        nospace.erase( std::remove_if( nospace.begin(), nospace.end(), ::isspace), nospace.end() );
        if (not(nospace.empty()))
        {
            values.push_back(convert_line_to_vector_of_doubles(current_line, separator));
            std::vector<std::string>::const_iterator it = titles.begin();
            size_t i = 0;
            for (;it != titles.end() ; ++it)
            {
                map[*it].push_back(values.back().at(i++));
            }
        }
    }
}

bool CSVFileReader::has_more_data() const
{
    return that_line != values.end();
}

std::vector<double> CSVFileReader::get_line()
{
    if (that_line == values.end())
    {
        THROW(__PRETTY_FUNCTION__, CSVFileReaderException, "No more lines in input CSV file.");
    }
    return *(that_line++);
}

std::map<std::string,std::vector<double> > CSVFileReader::get_map() const
{
    return map;
}
