#include "tcp.h"

#include <memory>
#include <string>
#include <vector>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

using boost::asio::const_buffer; using boost::asio::mutable_buffer;
using boost::asio::streambuf;
using boost::asio::async_write; using boost::asio::async_read;
using boost::future; using boost::promise;
using boost::system::error_code; using boost::system::system_error;

using net::Tcp;

using std::shared_ptr; using std::make_shared;
using std::string;
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

Tcp::Send_Return_t Tcp::send(const_buffer& req, error_code& ec)
{
    shared_ptr<promise<size_t>> prom = make_shared<promise<size_t>>();
    shared_ptr<future<size_t>> fut = std::make_shared<future<size_t>>(prom->get_future());

    async_write(socket, boost::asio::buffer(req), [prom](const error_code& ec, std::size_t len)
        { prom->set_value(len); }
    );

    return fut;
}

Tcp::Send_Return_t Tcp::send(vector<const_buffer>& req, error_code& ec)
{
    shared_ptr<promise<size_t>> prom = make_shared<promise<size_t>>();
    shared_ptr<future<size_t>> fut = std::make_shared<future<size_t>>(prom->get_future());

    async_write(socket, req, [prom](const error_code& ec, size_t len)
        { prom->set_value(len); }
    );

    return fut;
}

Tcp::Receive_Return_t Tcp::receive(error_code&)
{
    auto prom = make_shared<promise<const_buffer>>();
    auto fut = std::make_shared<future<const_buffer>>(prom->get_future());

    streambuf response;
    async_read(socket, response, [&prom, &response](const error_code& ec, size_t len)
        {
            // XXX
            // if (ec == boost::asio::error::eof) {
                // ec.clear();
            // }
            prom->set_value(response.data());
        }
    );

    return fut;
}
