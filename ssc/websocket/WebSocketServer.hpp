/*
 * WebSocketServer.hpp
 *
 * \date Apr 30, 2015
 * \author cec
 */


#ifndef WEBSOCKETSERVER_HPP_
#define WEBSOCKETSERVER_HPP_

#include <functional>

#include "ssc/websocket/WebSocketClient.hpp"
#include "ssc/websocket/WebSocketMessage.hpp"

namespace ssc
{
    namespace websocket
    {
        struct MessageHandler // What should we do when we receive a message?
        {
            virtual ~MessageHandler() {}
            virtual void operator()(const Message&) = 0;
        };

        class Server
        {
            public:
                Server(MessageHandler& message_handler, const short unsigned int port, const bool verbose = false);
                Server(const short unsigned int port, const bool verbose = false);
                ~Server();

            private:
                struct Impl;
                TR1(shared_ptr)<Impl> pimpl;
                Server();
        };
    }
}

#endif  /* WEBSOCKETSERVER_HPP_ */
