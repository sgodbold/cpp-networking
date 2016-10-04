#include "tcp.h"
#include "logger.h"

#include <functional>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "boost_config.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

using boost::asio::async_read;
using boost::asio::async_read_until;
using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::asio::transfer_exactly;
using boost::future;
using boost::promise;
using boost::system::error_code;
using boost::system::system_error;

using net::Tcp;

using std::bind;
using std::istream;
using std::shared_ptr;
using std::make_shared;
using std::move;
using std::mutex;
using std::string;
using std::unique_lock;
using std::vector;

namespace ph = std::placeholders;

// Starts connection with the server host
Tcp::Tcp(const std::string& host, const std::string& service)
    : connection_status(Status_t::Connecting),
      io_service(Io_Service_Manager::Behavior_t::Perpetual),
      socket(io_service.get())
{
    connect(host, service);
}

// Closes the connection if it is still open
Tcp::~Tcp()
{
    Logger::get()->trace("Tcp: destroying client");
    if (is_open())
    {
        close();
    }
}

bool Tcp::is_open()
{
    return connection_status == Status_t::Open and socket.is_open();
}

// Stops I/O service operations, closes out the socket, sets state to Closed.
// XXX split this up into close() and disconnect()? maybe somehow distinguish from client and
//     server disconnects
void Tcp::close()
{
    io_service.stop();

    try
    {
        // Shutdown before closing for portable graceful closures.
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
        socket.close();
    }
    catch(boost::system::system_error e)
    {
        Logger::get()->debug("Tcp::close() {} ", e.what());
    }

    connection_status = Status_t::Closed;
}

Tcp::Send_Return_t Tcp::send(const_buffer& req, error_code& ec)
{
    shared_ptr<promise<size_t>> prom = make_shared<promise<size_t>>();
    future<size_t> fut = prom->get_future();

    std::size_t size = boost::asio::buffer_size(req);
    async_write(socket, buffer(req, size), [this, prom](const error_code& ec, std::size_t len)
        {
            /* XXX Error: this lambda is executed by a io_service worker. When the service worker
                          calls handle_disconnect() this leads to a io_service_manager_stop()
                          ((still being run by the worker)) which calls worker_thread->join()
                          and that will seg fault.
            // XXX Maybe just set state of TCP to closed and let a non-worker thread deal with it?
            // XXX Signals? When there's many jobs in the io_service they should all be notified
                   of this disconnect. Somehow the jobs which can no longer execute must be stopped.
            */
            if (ec)
            {
                if (is_disconnect_error(ec))
                {
                    handle_disconnect();
                }
                else // XXX add more cases for all error codes
                {
                    prom->set_exception(ec);
                }
            }
            prom->set_value(len);
        }
    );

    return fut;
}

Tcp::Send_Return_t Tcp::send(vector<const_buffer>& req, error_code& ec)
{
    shared_ptr<promise<size_t>> prom = make_shared<promise<size_t>>();
    future<size_t> fut = prom->get_future();

    // XXX check ec. connection may be closed.
    async_write(socket, req, [this, prom](const error_code& ec, size_t len)
        {
            if (ec)
            {
                if (is_disconnect_error(ec))
                {
                    handle_disconnect();
                }
                else // XXX add more cases for all error codes
                {
                    prom->set_exception(ec);
                }
            }

            prom->set_value(len);
        }
    );

    return fut;
}

Tcp::Send_Return_t Tcp::send(const string& str, error_code& ec)
{
    // XXX don't send data that lives on the stack!
    const_buffer send_buf(buffer(str));
    return send(send_buf, ec);
}

Tcp::Send_Return_t Tcp::send(const int number, error_code& ec)
{
    // XXX don't send data that lives on the stack!
    string val = std::to_string(number);
    const_buffer send_buf(buffer(val));
    return send(send_buf, ec);
}

// XXX untested
Tcp::Receive_Return_t Tcp::receive(error_code& ec)
{
    auto prom = make_shared<promise<shared_ptr<string>>>();
    auto fut = prom->get_future();

    async_read(socket, receive_data, [this, prom] (const error_code& ec, size_t len)
               { handle_receive(ec, len, prom); }
    );

    return fut;
}

// XXX untested
Tcp::Receive_Return_t Tcp::receive(size_t size, error_code& ec)
{
    auto prom = make_shared<promise<shared_ptr<string>>>();
    auto fut = prom->get_future();

    async_read(socket, receive_data, transfer_exactly(size),
               [this, prom] (const error_code& ec, size_t len) { handle_receive(ec, len, prom); }
    );

    return fut;
}

// XXX untested
Tcp::Receive_Return_t Tcp::receive(std::string pattern, error_code& ec)
{
    auto prom = make_shared<promise<shared_ptr<string>>>();
    auto fut = prom->get_future();

    async_read_until(socket, receive_data, pattern, [this, prom](const error_code& ec, size_t len)
                     { handle_receive(ec, len, prom); }
    );

    return fut;
}

void Tcp::connect(const string& host, const string& service)
{
    // XXX use async_connect?

    // Get a list of endpoints corresponding to the server name.
    boost::asio::ip::tcp::resolver resolver(io_service.get());
    boost::asio::ip::tcp::resolver::query query(host, service);
    auto endpoint_iterator = resolver.resolve(query);

    // Try each endpoint until we successfully establish a connection.
    boost::asio::ip::tcp::resolver::iterator end;
    error_code error = boost::asio::error::host_not_found;
    while (error and endpoint_iterator != end)
    {
        socket.close();
        socket.connect(*endpoint_iterator++, error);
    }

    if (error)
    {
        Logger::get()->info("TCP connection failed: {}", error.message());

        connection_status = Status_t::Bad;
        throw system_error(error);
    }

    connection_status = Status_t::Open;
}

bool Tcp::is_disconnect_error(const error_code& ec)
{
    return (boost::asio::error::eof == ec) or (boost::asio::error::connection_reset == ec);
}

void Tcp::handle_disconnect()
{
    close();
}

void Tcp::handle_receive(const error_code& ec, size_t length,
                        shared_ptr<promise<shared_ptr<string>>> prom)
{
  Logger::get()->trace("Tcp::handle_receive");
    if (ec)
    {
        if (is_disconnect_error(ec))
        {
            // XXX self join error?
            handle_disconnect();
        }
        else // XXX add more cases for all error codes
        {
            prom->set_exception(ec);
        }
    }

    // Copy data out of input sequence
    istream is(&receive_data);
    auto res = make_shared<string>();
    is >> *res; // XXX will this work with binary data?

    Logger::get()->trace("Tcp::handle_receive length: {} message: '{}'", length, *res);

    prom->set_value(res);
}
