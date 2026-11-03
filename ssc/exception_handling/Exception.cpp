#include "ssc/exception_handling/Exception.hpp"
#include <sstream>

::ssc::exception_handling::Exception::Exception(const std::string& message, const std::string& file, const std::string& function, const unsigned int line):
        full_message(""),
        short_message(message)
{
    std::stringstream ss;
    ss << "In file " << file << ", line " << line << ", function " << function << ": " << short_message;
    full_message = ss.str();
}

::ssc::exception_handling::Exception::Exception(const Exception& rhs):
        full_message(rhs.full_message),
        short_message(rhs.short_message)
{
}

::ssc::exception_handling::Exception::~Exception () throw ()
{
}

const char* ::ssc::exception_handling::Exception::what() const throw()
{
    return full_message.c_str();
}

std::string ssc::exception_handling::Exception::get_message() const
{
    return short_message;
}
