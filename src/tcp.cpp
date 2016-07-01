#include "tcp.h"

#include <memory>
#include <string>
#include <vector>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

using boost::asio::const_buffer; using boost::asio::buffer; using boost::asio::streambuf;
using boost::asio::async_write; using boost::asio::async_read; using boost::asio::async_read_until;
using boost::future; using boost::promise;
using boost::system::error_code; using boost::system::system_error;

using net::Tcp;

using std::shared_ptr; using std::make_shared;
using std::string;
using std::vector;

Tcp::Tcp(const std::string& host, const std::string& service)
    : connection_status(Status_t::Connecting),
      io_service(Io_Service_Manager::Behavior_t::Perpetual), socket(io_service.get())
{
    connect(host, service);
}

Tcp::~Tcp()
{
    if(connection_status == Status_t::Open) {
        close();
    }
}

void Tcp::close()
{
    io_service.stop();

    // Shutdown before closing for portable graceful closures.
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
    socket.close();

    connection_status = Status_t::Closed;
}

Tcp::Send_Return_t Tcp::send(const_buffer& req, error_code& ec)
{
    shared_ptr<promise<size_t>> prom = make_shared<promise<size_t>>();
    future<size_t> fut = prom->get_future();

    std::size_t size = boost::asio::buffer_size(req);
    async_write(socket, buffer(req, size), [prom](const error_code& ec, std::size_t len)
        {
            prom->set_value(len);
        }
    );

    return fut;
}

Tcp::Send_Return_t Tcp::send(vector<const_buffer>& req, error_code& ec)
{
    shared_ptr<promise<size_t>> prom = make_shared<promise<size_t>>();
    future<size_t> fut = prom->get_future();

    async_write(socket, req, [prom](const error_code& ec, size_t len)
        { prom->set_value(len); }
    );

    return fut;
}

Tcp::Receive_Return_t Tcp::receive(error_code&)
{
    auto prom = make_shared<promise<shared_ptr<streambuf>>>();
    auto fut = prom->get_future();
    auto response = make_shared<streambuf>();

    async_read(socket, *response, [prom, response](const error_code& ec, size_t len)
        {
            // XXX check errors
            prom->set_value(response);
        }
    );

    return fut;
}

Tcp::Receive_Return_t Tcp::receive(size_t size, error_code& ec)
{
    auto prom = make_shared<promise<shared_ptr<streambuf>>>();
    auto fut = prom->get_future();
    auto response = make_shared<streambuf>(size);

    async_read(socket, *response, [prom, response](const error_code& ec, size_t len)
        {
            // XXX check errors
            prom->set_value(response);
        }
    );

    return fut;
}

Tcp::Receive_Return_t Tcp::receive_line(error_code& ec)
{
    auto prom = make_shared<promise<shared_ptr<streambuf>>>();
    auto fut = prom->get_future();
    auto response = make_shared<streambuf>();

    async_read_until(socket, *response, '\n', [prom, response](const error_code& ec, size_t len)
        {
            // XXX check errors
            prom->set_value(response);
        }
    );

    return fut;
}

/* Private */

void Tcp::connect(const string& host, const string& service)
{
    // XXX use async_connect

    // Get a list of endpoints corresponding to the server name.
    boost::asio::ip::tcp::resolver resolver(io_service.get());
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
        connection_status = Status_t::Bad;
        throw system_error(error);
    }

    connection_status = Status_t::Open;
}
