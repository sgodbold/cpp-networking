#include "tcp.h"

#include <boost/asio.hpp>
#include <string>
#include <vector>

using boost::asio::const_buffer; using boost::asio::streambuf;
using boost::system::error_code; using boost::system::system_error;

net::tcp::tcp(const std::string& host, const std::string& service)
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

void net::tcp::send(const_buffer& req)
{
    boost::asio::write(socket, req);
}

void net::tcp::send(std::vector<const_buffer> req)
{
    boost::asio::write(socket, req);
}

const_buffer net::tcp::receive()
{
    streambuf response;
    error_code error;
    boost::asio::read(socket, response, error);

    // EOF error is good; everything else is bad.
    if (error and error != boost::asio::error::eof) {
        throw system_error(error);
    }

    return response.data();
}

// Receive data from connection. Calls h with a response
// buffer on success and throws on error.
void net::tcp::async_receive(Receive_Handler h)
{
    streambuf response;

    boost::asio::async_read(socket, response,
        [h, &response](error_code& error, std::size_t len)
        {
            // EOF error is good; everything else is bad.
            if(error and error != boost::asio::error::eof) {
                throw system_error(error);
            }
            h(response.data());
        }
    );
}
