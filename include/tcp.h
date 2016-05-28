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
    using Send_Return_t = std::shared_ptr<boost::future<size_t>>;
    using Receive_Return_t = boost::future<std::shared_ptr<boost::asio::streambuf>>;

    // XXX blocking
    explicit Tcp(const std::string& host, const std::string& service);

    // XXX blocking
    ~Tcp();

    Send_Return_t send(boost::asio::const_buffer&, boost::system::error_code&);
    Send_Return_t send(std::vector<boost::asio::const_buffer>&, boost::system::error_code&);

    // Receive until an error occurs
    Receive_Return_t receive(boost::system::error_code&);

    // Receive a specific number of bytes
    Receive_Return_t receive(size_t, boost::system::error_code&);

    // Receive until a newline character
    Receive_Return_t receive_line(boost::system::error_code&);

private:
    static void add_io_thread();

    static boost::asio::io_service io_service;
    static std::shared_ptr<boost::asio::io_service::work> io_work;
    static std::vector<boost::thread> io_threads;

    boost::asio::ip::tcp::socket socket;

}; // Tcp
} // net

#endif
