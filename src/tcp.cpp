#include "tcp.h"

#include <boost/asio.hpp>
#include <string>
#include <vector>

using boost::asio::const_buffer; using boost::asio::streambuf;
using boost::system::error_code; using boost::system::system_error;

using async_net::Tcp;

Tcp::Tcp(const std::string& host, const std::string& service)
    : socket(io_service)
{
    // Get a list of endpoints corresponding to the server name.
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(host, service);
    auto endpoint_iterator = resolver.resolve(query);

    // Try each endpoint until we successfully establish a connection.
    boost::asio::ip::tcp::resolver::iterator end;
    error_code error = boost::asio::error::host_not_found;
    while (error and endpoint_iterator != end) {
        socket.close();
        socket.connect(*endpoint_iterator++, error);
    }
    if (error) {
        throw system_error(error);
    }
}

Tcp::~Tcp()
{
    // Shutdown before closing for portable graceful closures.
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
    socket.close();
}

// Send buffer contents to the connection. Calls h with any errors
// once complete.
void Tcp::send(const_buffer& req, Send_Handler_t h)
{
    boost::asio::async_write(socket, req,
        [h](error_code& error, std::size_t len)
        { h(error); }
    );
}

// Send sequence of buffers contents to the connection. Calls h with
// any errors once complete.
void Tcp::send(std::vector<const_buffer>& req, Send_Handler_t h)
{
    boost::asio::async_write(socket, req,
        [h](error_code& error, std::size_t len)
        { h(error); }
    );
}

// Receive data from connection. Calls h with a response
// buffer on success and throws on error.
void Tcp::receive(Receive_Handler_t h)
{
    streambuf response;
    boost::asio::async_read(socket, response,
        [h, &response](error_code& error, std::size_t len)
        {
            // EOF error is good so clear it before calling handler.
            if (error == boost::asio::error::eof) {
                error.clear();
            }
            h(error, response.data());
        }
    );
}
