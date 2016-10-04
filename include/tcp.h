#ifndef CPP_NETWORKING_TCP_H
#define CPP_NETWORKING_TCP_H

#include "io_service_manager.h"
#include "logger.h"

#include <mutex>
#include <string>
#include <vector>

#include "boost_config.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

namespace net {

class Tcp
{
    public:
        using Send_Return_t = boost::future<size_t>;
        using Receive_Return_t = boost::future<std::shared_ptr<std::string>>;

        enum class Status_t
        {
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

        bool is_open();

        void close();

        /* Sending Data:
         *
         * All send operations are asynchronous and immediately return a future containing size sent.
         *
         * Make sure that the data being sent will continue to exist as long as
         * the operation continues.
         *
         * TODO: test error codes
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
         */
        Receive_Return_t receive(boost::system::error_code&);
        Receive_Return_t receive(size_t size, boost::system::error_code&);
        Receive_Return_t receive(std::string pattern, boost::system::error_code&);

    private:
        // XXX blocking
        void connect(const std::string& host, const std::string& service);

        bool is_disconnect_error(const boost::system::error_code& ec);

        void handle_disconnect();

        void handle_receive(const boost::system::error_code& ec, size_t length,
                            std::shared_ptr<boost::promise<std::shared_ptr<std::string>>> prom);

        Status_t connection_status;
        Io_Service_Manager io_service;

        boost::asio::ip::tcp::socket socket;

        /* Common receive buffer rational:
         * 
         * Problem:
         * All forms of receive operations must be called using the same buffer. This is necessary
         * because the different read variants (error, pattern, size) are all implemented using
         * async_read_some. This implementation reads in chunks at a time, and if a delimeter or
         * size limit occurs in that chunk THEN it stops, after having already read data past it.
         *
         * Solution:
         * Receiving on one buffer solves this. Read data past where the previous receive function
         * returned still exists in the buffer. That data is consumed first then more data is read
         * in from the socket.
         *
         */
        boost::asio::streambuf receive_data;

}; // Tcp

} // net

#endif
