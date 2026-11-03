/*
 * WebSocketObserverException.hpp
 *
 * \date Apr 30, 2015
 * \author cec
 */


#ifndef WEBSOCKETOBSERVEREXCEPTION_HPP_
#define WEBSOCKETOBSERVEREXCEPTION_HPP_

#include "ssc/exception_handling/Exception.hpp"

namespace ssc
{
    namespace websocket
    {
        class WebSocketException: public ::ssc::exception_handling::Exception
        {
            public:
                WebSocketException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };
    }
}

#endif  /* WEBSOCKETOBSERVEREXCEPTION_HPP_ */
