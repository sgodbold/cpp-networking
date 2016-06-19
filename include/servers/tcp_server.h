#ifndef CPP_NETWORKING_TCP_SERVER_H
#define CPP_NETWORKING_TCP_SERVER_H

#include "servers/tcp_base_session.h"

#include <iostream>
#include <memory>
#include <string>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace net {

class Tcp_Server
{
public:
    enum class Role_t
    {
        // A simple server accepts any incoming connection but does nothing
        // after that. Useful for testing
        Passive,

        // Reads incoming messages one line at a time and echos them back.
        Echo,
    };

    Tcp_Server(Role_t, short port);
    ~Tcp_Server();

private:
    void do_accept();

    void new_connection(boost::asio::ip::tcp::socket);

    // XXX move this to it's own class
    void add_io_thread();

    const Role_t server_role;

    // XXX move to separate class
    boost::asio::io_service io_service;
    std::shared_ptr<boost::asio::io_service::work> io_work;
    std::vector<boost::thread> io_threads;
    std::shared_ptr<boost::thread> accept_thread;

    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;

}; // Tcp_Server

} // net

#endif
