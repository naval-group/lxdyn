/*
 * WebSocketServer.cpp
 *
 * \date Apr 30, 2015
 * \author cec
 */

#include "websocketpp/common/thread.hpp"
#include "ssc/websocket/WebSocketServer.hpp"
#include "ssc/websocket/WebSocketException.hpp"
#include "WebSocketMessageImpl.hpp"

using namespace ssc::websocket;

struct Server::Impl
{
    Impl() :
        server(),
        server_thread()
    {}

    WSServer server;
    websocketpp::lib::thread server_thread; // Thread in which the server runs
};



typedef std::function<void(WSServer* , websocketpp::connection_hdl, message_ptr )> InternalMessageHandler;

using std::placeholders::_1;
using std::placeholders::_2;

void create_server(WSServer& server, const short unsigned int port, const InternalMessageHandler& message_handler, const bool verbose);

InternalMessageHandler get_lambda(MessageHandler& message_handler);
InternalMessageHandler get_lambda(MessageHandler& message_handler)
{
    return [&message_handler](WSServer* server, const websocketpp::connection_hdl& handle, const message_ptr& mes )
                                                                {
                                                                 WebSocketMessageImpl pimpl;
                                                                 pimpl.handle = handle;
                                                                 pimpl.message = mes;
                                                                 pimpl.server = server;
                                                                 message_handler(Message(pimpl));};
}

Server::Server(MessageHandler& message_handler, const short unsigned int port, const bool verbose):
        pimpl(new Impl())
{
    pimpl->server_thread = websocketpp::lib::thread(create_server, std::ref(pimpl->server), port, get_lambda(message_handler), verbose);
}

struct DoNothing : public MessageHandler
{
    void operator()(const Message&){}
};

Server::Server(const short unsigned int port, const bool verbose) : pimpl(new Impl())
{
    DoNothing message_handler;
    pimpl->server_thread = websocketpp::lib::thread(create_server, std::ref(pimpl->server), port, get_lambda(message_handler), verbose);
}


Server::~Server()
{
    pimpl->server.stop();
    pimpl->server_thread.join();
}


void create_server(WSServer& server, const short unsigned int port, const InternalMessageHandler& message_handler, const bool verbose)
{
    try
    {
        server.set_reuse_addr(true);
        // Set logging settings
        server.clear_access_channels(websocketpp::log::alevel::all);
        if (verbose)
        {
            server.set_access_channels(websocketpp::log::alevel::all);
        }
        // Initialize ASIO
        server.init_asio();
        // Register our message handler
        server.set_message_handler(std::bind(message_handler,&server,::_1,::_2));
        // Listen on port
        server.listen(port);
        // Start the server accept loop
        server.start_accept();
        // Start the ASIO io_service run loop
        server.run();
    }
    catch (const websocketpp::exception& e)
    {
        THROW(__PRETTY_FUNCTION__, ssc::websocket::WebSocketException, "There was a problem establishing the websocket connexion: " << e.what() << std::endl);
    }
}
