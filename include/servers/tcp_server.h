#ifndef CPP_NETWORKING_TCP_SERVER_H
#define CPP_NETWORKING_TCP_SERVER_H

#include "io_service_manager.h"
#include "servers/tcp_base_session.h"

#include <memory>
#include <string>

#include "boost_config.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace net {

class Tcp_Server
{
public:
    enum class Role_t
    {
        // A server which receives until a newline character '\n' and then immediately writes
        // it back to the client
        Echo,
    };

    Tcp_Server(Role_t, short port);
    ~Tcp_Server();

    // Stop the server. No more new connections are accepted. XXX do running sessions still continue?
    void stop();

private:
    // Async accept loop that creates a new session for every connected client.
    void do_accept();

    // Creates and starts a new connection session of type server_role.
    void new_connection(boost::asio::ip::tcp::socket);

    // Server properties
    const Role_t server_role;
    bool running;

    // Async objects
    Io_Service_Manager io_service;
    std::shared_ptr<boost::thread> accept_thread;

    // Networking objects
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;

}; // Tcp_Server

} // net

#endif
