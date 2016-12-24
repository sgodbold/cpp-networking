#ifndef CPP_NETWORKING_TCP_H
#define CPP_NETWORKING_TCP_H

#include "io_service_manager.h"
#include "logger.h"

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
        using Send_Prom_t = std::shared_ptr<boost::promise<size_t>>;
        using Receive_Prom_t = std::shared_ptr<boost::promise<std::shared_ptr<std::string>>>;
        using Send_Callback_t = std::function<void(const boost::system::error_code&, size_t)>;
        using Receive_Callback_t = std::function<void(const boost::system::error_code&, size_t)>;

        // XXX blocking
        void connect(const std::string& host, const std::string& service);

        // Post a send / receive function to socket_rw_strand. The passed in function must
        // accept a Send/Receive_Callback_t. The callback is used to return socket data
        // back to the future given to the caller.
        Send_Return_t post_send_to_strand(std::function<void(Send_Callback_t)> send_fn);
        Receive_Return_t post_recv_to_strand(std::function<void(Receive_Callback_t)> recv_fn);
        
        void handle_send(const boost::system::error_code& ec, size_t length, Send_Prom_t prom);
        void handle_receive(const boost::system::error_code& ec, size_t length, Receive_Prom_t prom);
        void handle_disconnect();

        bool is_disconnect_error(const boost::system::error_code& ec);

        // Reads length bytes from receive_data into a string. Only call this in an async_read
        // callback when you can guaruntee that length data is in the buffer.
        std::shared_ptr<std::string> consume_receive_data(size_t length);

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
        Io_Service_Manager io_service;
        boost::asio::ip::tcp::socket socket;
        boost::asio::io_service::strand socket_rw_strand;
        Status_t connection_status;

}; // Tcp

} // net

#endif
