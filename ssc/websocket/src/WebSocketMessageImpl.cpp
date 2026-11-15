/*
 * WebSocketMessageImpl.cpp
 *
 * \date May 5, 2015
 * \author cec
 */


#include "WebSocketMessageImpl.hpp"

WebSocketMessageImpl::WebSocketMessageImpl() = default;

WebSocketMessageImpl::~WebSocketMessageImpl()= default;

WebSocketMessageImpl::WebSocketMessageImpl(const WebSocketMessageImpl& rhs) = default;

WebSocketMessageImpl& WebSocketMessageImpl::operator=(const WebSocketMessageImpl& rhs)
{
    if (&rhs != this)
    {
        server = rhs.server;
        handle = rhs.handle;
        message = rhs.message;
    }
    return *this;
}



