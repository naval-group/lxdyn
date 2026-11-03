/*
 * VectorOfEquallySpacedNumberException.hpp
 *
 * \date 12 juin 2013, 15:47:26
 * \author cec
 */

#ifndef VECTOROFEQUALLYSPACEDNUMBEREXCEPTION_HPP_
#define VECTOROFEQUALLYSPACEDNUMBEREXCEPTION_HPP_

#include "ssc/exception_handling/Exception.hpp"

namespace ssc
{
    namespace interpolation
    {
        class VectorOfEquallySpacedNumbersException: public ::ssc::exception_handling::Exception
        {
            public:
                VectorOfEquallySpacedNumbersException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line) {
                }
        };
    }
}

#endif  /* VECTOROFEQUALLYSPACEDNUMBEREXCEPTION_HPP_ */
