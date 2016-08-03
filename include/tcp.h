#ifndef CPP_NETWORKING_TCP_H
#define CPP_NETWORKING_TCP_H

#include "io_service_manager.h"

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

    // XXX replace with specific state checks
    Status_t status() { return connection_status; }

    void close();

    /* Sending Data:
     *
     * All send operations are asynchronous and immediately return a future containing size sent.
     *
     * Make sure that the data being sent will continue to exist as long as
     * the operation continues.
     *
     * TODO: set error codes
     */
    Send_Return_t send(boost::asio::const_buffer&, boost::system::error_code&);
    Send_Return_t send(std::vector<boost::asio::const_buffer>&, boost::system::error_code&);
    Send_Return_t send(const std::string&, boost::system::error_code&);
    Send_Return_t send(const int, boost::system::error_code&);

    /* Receiving Data:
     *
     * All receive operations are asynchronous and immediately return a future containing data.
     *
     * There are 3 core receive operations:
     * 1. Receive until an error occurs (such as EOF)
     * 2. Recieve a length of data
     * 3. Recieve until a pattern
     *
     * TODO: set error codes
     */
    Receive_Return_t receive(boost::system::error_code&);
    Receive_Return_t receive(size_t size, boost::system::error_code&);
    Receive_Return_t receive(std::string pattern, boost::system::error_code&);

private:
    // XXX blocking
    void connect(const std::string& host, const std::string& service);

    Status_t connection_status;
    Io_Service_Manager io_service;

    boost::asio::ip::tcp::socket socket;

}; // Tcp
} // net

#endif
