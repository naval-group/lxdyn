/*
 * SplinesException.hpp
 *
 * \date 13 juin 2013, 11:04:40
 * \author cec
 */

#ifndef SPLINESEXCEPTION_HPP_
#define SPLINESEXCEPTION_HPP_

#include "ssc/exception_handling/Exception.hpp"

namespace ssc
{
    namespace interpolation
    {
        class SplinesException : public ::ssc::exception_handling::Exception
        {
            public:
                SplinesException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };
    }
}

#endif  /* SPLINESEXCEPTION_HPP_ */
