/*
 * SplinesException.hpp
 *
 * \date 13 juin 2013, 11:04:40
 * \author cec
 */

#ifndef PIECEWISECONSTANTEXCEPTION_HPP_
#define PIECEWISECONSTANTEXCEPTION_HPP_

#include "ssc/exception_handling/Exception.hpp"

namespace ssc
{
    namespace interpolation
    {
        class PiecewiseConstantException : public ::ssc::exception_handling::Exception
        {
            public:
                PiecewiseConstantException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };
    }
}

#endif  /* PIECEWISECONSTANTEXCEPTION_HPP_ */
