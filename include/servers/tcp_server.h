#ifndef CPP_NETWORKING_TCP_SERVER_H
#define CPP_NETWORKING_TCP_SERVER_H

#include <memory>

#include <boost/asio.hpp>

// template <typename T>
class Tcp_Server
{
public:
    Tcp_Server(boost::asio::io_service& io_service, short port);

private:
    virtual void do_accept();

    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
};

#endif
