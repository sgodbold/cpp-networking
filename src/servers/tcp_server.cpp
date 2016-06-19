#include "servers/tcp_server.h"

#include "servers/tcp_echo_session.h"
#include "servers/tcp_passive_session.h"

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

using net::Tcp_Server;

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::system::error_code;

using std::make_shared;
using std::shared_ptr;
using std::string;

Tcp_Server::Tcp_Server(Role_t role, short port)
    : server_role(role), io_service(), // io_work(io_service),
      acceptor(io_service, tcp::endpoint(tcp::v4(), port)), socket(io_service)
{
    add_io_thread();
    accept_thread = make_shared<boost::thread>(&Tcp_Server::do_accept, this);
}

Tcp_Server::~Tcp_Server()
{
    // Stop all asynchronous operations
    io_service.stop();
    io_work.reset();

    // Stop all threads running
    for_each(io_threads.begin(), io_threads.end(), [](boost::thread& t) { t.join(); });
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

void Tcp_Server::add_io_thread()
{
    io_work = make_shared<boost::asio::io_service::work>(io_service);
    io_threads.push_back(boost::thread([&]() {
        Tcp_Server::io_service.run();
    }));
}
