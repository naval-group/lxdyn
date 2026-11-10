/*
 * WebSocketMessageImpl.hpp
 *
 * \date May 5, 2015
 * \author cec
 */


#ifndef WEBSOCKETMESSAGEIMPL_HPP_
#define WEBSOCKETMESSAGEIMPL_HPP_

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
using WSServer = websocketpp::server<websocketpp::config::asio>;
using message_ptr = WSServer::message_ptr;

struct WebSocketMessageImpl
{
    WebSocketMessageImpl();
    ~WebSocketMessageImpl();
    WebSocketMessageImpl(const WebSocketMessageImpl& rhs);
    WebSocketMessageImpl& operator=(const WebSocketMessageImpl& rhs);


    WSServer* server;
    websocketpp::connection_hdl handle;
    message_ptr message;
};

#endif  /* WEBSOCKETMESSAGEIMPL_HPP_ */
