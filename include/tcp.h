#ifndef CPP_NETWORKING_TCP_H
#define CPP_NETWORKING_TCP_H

#include "io_service.h"

#include <string>
#include <vector>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

namespace net {

class Tcp {
public:
    using Send_Return_t = boost::future<size_t>;
    using Receive_Return_t = boost::future<std::shared_ptr<boost::asio::streambuf>>;

    enum class Status_t {
        Connecting,
        Open,
        Closed,
        Bad,
    };

    // XXX blocking
    explicit Tcp(const std::string& host, const std::string& service);

    // XXX blocking
    ~Tcp();

    Status_t status() { return connection_status; }

    void close();

    // Send a buffer
    Send_Return_t send(boost::asio::const_buffer&, boost::system::error_code&);

    // Send multiple buffers
    Send_Return_t send(std::vector<boost::asio::const_buffer>&, boost::system::error_code&);

    // XXX templated send?? enable sending strings, numbers, etc

    // Receive until an error occurs
    Receive_Return_t receive(boost::system::error_code&);

    // Receive a specific number of bytes
    Receive_Return_t receive(size_t, boost::system::error_code&);

    // Receive until a newline character
    Receive_Return_t receive_line(boost::system::error_code&);

private:
    // XXX blocking
    void connect(const std::string& host, const std::string& service);

    Status_t connection_status;
    Io_Service io_service;

    boost::asio::ip::tcp::socket socket;

}; // Tcp
} // net

#endif
