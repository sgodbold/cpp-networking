#ifndef CPP_NETWORKING_TCP
#define CPP_NETWORKING_TCP

#include <boost/asio.hpp>
#include <string>
#include <vector>

namespace async_net {

class Tcp {
public:
    using Receive_Handler = void(*)(boost::system::error_code& errror,
                                    boost::asio::const_buffer response);
    using Send_Handler = void(*)(boost::system::error_code& error);

    Tcp(const std::string& host, const std::string& service);
    ~Tcp();

    void send(boost::asio::const_buffer& req, Send_Handler);
    void send(std::vector<boost::asio::const_buffer>& req, Send_Handler);

    void receive(Receive_Handler);

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket;

}; // Tcp
} // net

#endif
