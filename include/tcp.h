#ifndef CPP_NETWORKING_TCP
#define CPP_NETWORKING_TCP

#include <boost/asio.hpp>
#include <string>
#include <vector>

namespace net {

class tcp {
public:
    using Receive_Handler = void(*)(boost::asio::const_buffer response);

    tcp(const std::string& host, const std::string& service);

    void send(boost::asio::const_buffer& req);
    void send(std::vector<boost::asio::const_buffer> req);

    boost::asio::const_buffer receive();
    void async_receive(Receive_Handler h);

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket;

}; // tcp
} // net

#endif
