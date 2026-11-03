/*
 * ParabolicInterpolationException.hpp
 *
 * \date 12 juin 2013, 16:14:58
 * \author cec
 */

#ifndef PARABOLICINTERPOLATIONEXCEPTION_HPP_
#define PARABOLICINTERPOLATIONEXCEPTION_HPP_

#include "ssc/exception_handling/Exception.hpp"

namespace ssc
{
    namespace interpolation
    {
        class ParabolicInterpolationException : public ::ssc::exception_handling::Exception
        {
            public:
                ParabolicInterpolationException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };
    }
}


#endif  /* PARABOLICINTERPOLATIONEXCEPTION_HPP_ */
