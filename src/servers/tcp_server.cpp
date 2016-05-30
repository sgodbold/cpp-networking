#include "servers/tcp_server.h"

#include "servers/tcp_echo_session.h"

#include <iostream>
#include <string>

#include <boost/asio.hpp>

using net::Tcp_Server;

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::system::error_code;

using std::make_shared;
using std::shared_ptr;
using std::string;

Tcp_Server::Tcp_Server(Role_t role, io_service& io_service_, short port)
    : server_role(role), acceptor(io_service_, tcp::endpoint(tcp::v4(), port)),
      socket(io_service_)
{
    do_accept();
}

void Tcp_Server::do_accept()
{
    std::cout << "Waiting for connection..." << std::endl;
    acceptor.async_accept(socket, [this](error_code ec)
        {
            std::cout << "Accepted" << std::endl;
            if (!ec) {
                new_connection(std::move(socket));
            }

            // Call self again for the next incoming connection
            do_accept();
        }
    );
}

void Tcp_Server::new_connection(boost::asio::ip::tcp::socket s)
{
    switch(server_role) {
        case Role_t::Echo:
            make_shared<net::Tcp_Echo_Session>(std::move(s))->start();
            break;
        case Role_t::Passive:
            break;
        default:
            std::cerr << "Invalid Tcp_Server role" << std::endl;
    };

    std:: cout << "Session Created" << std::endl;
}
