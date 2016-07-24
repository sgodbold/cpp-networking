#include "servers/tcp_server.h"

#include "io_service_manager.h"
#include "servers/tcp_echo_session.h"
#include "servers/tcp_passive_session.h"

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

using net::Io_Service_Manager;
using net::Tcp_Server;

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::system::error_code;

using std::make_shared;
using std::shared_ptr;
using std::string;

Tcp_Server::Tcp_Server(Role_t role, short port)
    : server_role(role), io_service(Io_Service_Manager::Behavior_t::Perpetual),
      acceptor(io_service.get(), tcp::endpoint(tcp::v4(), port)), socket(io_service.get())
{
    accept_thread = make_shared<boost::thread>(&Tcp_Server::do_accept, this);
}

Tcp_Server::~Tcp_Server()
{
    io_service.stop();

    accept_thread->join();

    // Release socket
    boost::system::error_code ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
    socket.close();

    std::cout << "Server Stopped" << std::endl;
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
        case Role_t::Passive:
            make_shared<net::Tcp_Passive_Session>(std::move(s))->start();
            break;
        case Role_t::Echo:
            make_shared<net::Tcp_Echo_Session>(std::move(s))->start();
            break;
        default:
            std::cerr << "Invalid Tcp_Server role" << std::endl;
    };

    std:: cout << "Session Created" << std::endl;
}
