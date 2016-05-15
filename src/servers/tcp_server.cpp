#include "servers/tcp_server.h"

#include "servers/tcp_session_echo.h"

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::system::error_code;

using std::shared_ptr;
using std::make_shared;

Tcp_Server::Tcp_Server(io_service& io_service_, short port)
    : acceptor(io_service_, tcp::endpoint(tcp::v4(), port)), socket(io_service_)
{
    do_accept();
}

void Tcp_Server::do_accept()
{
    std::cout << "Waiting for connection..." << std::endl;
    acceptor.async_accept(socket, [this](error_code ec)
        {
            std::cout << "Accepted" << std::endl;
            // Create a new tcp connection for this client
            if (!ec) {
                // XXX template this
                make_shared<Tcp_Session_Echo>(std::move(socket))->start();
                std:: cout << "Session Created" << std::endl;
            }

            // Call self again for the next incoming connection
            do_accept();
        }
    );
}
