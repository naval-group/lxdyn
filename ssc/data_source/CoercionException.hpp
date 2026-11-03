/*
 * CoercionException.hpp
 *
 * \date 18 june 2013, 14:37:43
 * \author cec
 */

#ifndef COERCIONEXCEPTION_HPP_
#define COERCIONEXCEPTION_HPP_

#include "ssc/exception_handling/Exception.hpp"

namespace ssc
{
    namespace data_source
    {
        class CoercionException : public ::ssc::exception_handling::Exception
        {
            public:
                CoercionException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                        Exception(message, file, function, line)
                {
                }
        };
    }
}


#endif  /* COERCIONEXCEPTION_HPP_ */
