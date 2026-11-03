#ifndef JSONEXCEPTIONHPP
#define JSONEXCEPTIONHPP

#include "ssc/exception_handling/Exception.hpp"

namespace ssc
{
    namespace json
    {
        class Exception: public ssc::exception_handling::Exception
        {
            public:
                Exception(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                       ssc::exception_handling::Exception::Exception(message, file, function, line)
                {
                }
        };
    }
}

#endif
