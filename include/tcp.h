#ifndef CPP_NETWORKING_TCP_H
#define CPP_NETWORKING_TCP_H

#include <string>
#include <vector>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

namespace net {

class Tcp {
public:
    explicit Tcp(const std::string& host, const std::string& service);

    ~Tcp();

    boost::future<size_t> send(boost::asio::const_buffer&, boost::system::error_code&);
    boost::future<size_t> send(std::vector<boost::asio::const_buffer>&, boost::system::error_code&);

    boost::future<boost::asio::const_buffer> receive(boost::system::error_code&);

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket;

}; // Tcp
} // net

#endif
