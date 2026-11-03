#ifndef TEXTFILEREADEREXCEPTION
#define TEXTFILEREADEREXCEPTION

#include "ssc/exception_handling/Exception.hpp"

class TextFileReaderException: public ::ssc::exception_handling::Exception
{
    public:
        TextFileReaderException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                                        Exception(message, file, function, line)
        {
        }
};


#endif
