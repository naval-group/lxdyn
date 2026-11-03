/*
 * KinematicsException.hpp
 *
 * \date 7 avr. 2014
 * \author cec
 */

#ifndef KINEMATICSEXCEPTION_HPP_
#define KINEMATICSEXCEPTION_HPP_

#include "ssc/exception_handling/Exception.hpp"

namespace ssc
{
    namespace kinematics
    {
        class KinematicsException: public ::ssc::exception_handling::Exception
        {
            public:
                KinematicsException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };
    }
}

#endif /* KINEMATICSEXCEPTION_HPP_ */
