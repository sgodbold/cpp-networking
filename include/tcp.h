#ifndef CPP_NETWORKING_TCP
#define CPP_NETWORKING_TCP

#include <boost/asio.hpp>
#include <string>
#include <vector>

namespace async_net {

class Tcp {
public:
    using Receive_Handler_t = std::function<void(boost::system::error_code&,
                                                 boost::asio::const_buffer response)>;
    using Send_Handler_t = std::function<void(boost::system::error_code&)>;

    explicit Tcp(const std::string& host, const std::string& service);

    ~Tcp();

    void send(boost::asio::const_buffer& req, Send_Handler_t);
    void send(std::vector<boost::asio::const_buffer>& req, Send_Handler_t);

    void receive(Receive_Handler_t);

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket;

}; // Tcp
} // net

#endif
