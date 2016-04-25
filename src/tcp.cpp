#include "tcp.h"

#include <string>
#include <vector>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

using boost::asio::const_buffer; using boost::asio::streambuf;
using boost::system::error_code; using boost::system::system_error;
using boost::asio::async_write; using boost::asio::async_read;
using boost::future;
using boost::promise;

using net::Tcp;

using std::vector;

Tcp::Tcp(const std::string& host, const std::string& service)
    : socket(io_service)
{
    // XXX use async_connect

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

future<size_t> Tcp::send(const_buffer& req, error_code& ec)
{
    promise<size_t> prom;
    future<size_t> fut = prom.get_future();

    async_write(socket, req, [&prom](const error_code& ec, std::size_t len)
        { prom.set_value(len); }
    );

    return fut;
}

future<size_t> Tcp::send(vector<const_buffer>& req, error_code& ec)
{
    promise<size_t> prom;
    future<size_t> fut = prom.get_future();

    async_write(socket, req, [&prom](const error_code& ec, size_t len)
        { prom.set_value(len); }
    );

    return fut;
}

future<const_buffer> Tcp::receive(error_code&)
{
    promise<const_buffer> prom;
    future<const_buffer> fut = prom.get_future();

    streambuf response;
    async_read(socket, response, [&prom, &response](const error_code& ec, size_t len)
        {
            // XXX
            // if (ec == boost::asio::error::eof) {
                // ec.clear();
            // }
            // len doesn't matter; caller can get size of response buffer
            prom.set_value(response.data());
        }
    );

    return fut;
}
