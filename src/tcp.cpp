#include "tcp.h"
#include "logger.h"

#include <algorithm>        // copy
#include <functional>
#include <iterator>         // back_inserter
#include <memory>
#include <string>
#include <vector>

#include "boost_config.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

using boost::future;
using boost::promise;
using boost::asio::async_read;
using boost::asio::async_read_until;
using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::buffers_begin;
using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::asio::transfer_exactly;
using boost::system::error_code;
using boost::system::system_error;

using net::Tcp;

using std::copy;
using std::shared_ptr;
using std::make_shared;
using std::move;
using std::string;
using std::vector;

namespace ph = std::placeholders;

// Starts connection with the server host
Tcp::Tcp(const std::string& host, const std::string& service)
    : connection_status(Status_t::Connecting),
      io_service(Io_Service_Manager::Behavior_t::Perpetual),
      socket(io_service.get()),
      async_receive_strand(io_service.create_strand()),
      async_receive_handle_strand(io_service.create_strand())
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
    auto receive_callback = [this, prom] (const error_code& ec, size_t len)
         { handle_receive(ec, len, prom); };

    async_receive_strand.post([this, receive_callback] ()
        { async_read(socket, receive_data, receive_callback); }
    );

    return fut;
}

Tcp::Receive_Return_t Tcp::receive(size_t size, error_code& ec)
{
    auto prom = make_shared<promise<shared_ptr<string>>>();
    auto fut = prom->get_future();
    auto receive_callback = [this, prom] (const error_code& ec, size_t len)
        { handle_receive(ec, len, prom); };

    async_receive_strand.post([this, size, receive_callback] ()
        {
            async_read(socket, receive_data, transfer_exactly(size),
                       receive_callback);
        }
    );

    return fut;
}

Tcp::Receive_Return_t Tcp::receive(std::string pattern, error_code& ec)
{
    /*
    auto prom = make_shared<promise<shared_ptr<string>>>();
    auto fut = prom->get_future();
    auto receive_callback = [this, prom] (const error_code& ec, size_t len)
        { handle_receive(ec, len, prom); };

    async_receive_strand.post([this, pattern, receive_callback] ()
            { async_read_until(socket, receive_data, pattern, receive_callback); }
    );
    */

    // return fut;
    
    return receive_impl([this, pattern](Receive_Callback_t callback)
                        { async_read_until(socket, receive_data, pattern, callback); }
    );
}

Tcp::Receive_Return_t Tcp::receive_impl(std::function<void(Receive_Callback_t)> recv_fn)
{
    auto prom = make_shared<promise<shared_ptr<string>>>();
    auto fut = prom->get_future();

    auto recv_callback = [this, prom] (const error_code& ec, size_t len)
        { handle_receive(ec, len, prom); };

    async_receive_strand.post(bind(recv_fn, recv_callback));

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

void Tcp::handle_receive(const error_code& ec, size_t length, Receive_Prom_t prom)
{
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

    shared_ptr<string> response = consume_receive_data(length);

    prom->set_value(response);
}

shared_ptr<string> Tcp::consume_receive_data(size_t length)
{
    Logger::get()->trace("Tcp::handle_receive pre-read buffer size: {}", receive_data.size());

    auto res = make_shared<string>();
    res->reserve(length);

    // Retrieve socket input buffers and copy data into the return string.
    // Once copy is done remove the data from the socket buffer.
    auto recv_bufs = receive_data.data();
    std::copy(boost::asio::buffers_begin(recv_bufs),
              boost::asio::buffers_begin(recv_bufs) + length,
              std::back_inserter(*res));
    receive_data.consume(length);

    Logger::get()->trace("Tcp::handle_receive post-read buffer size: {}", receive_data.size());

    return res;
}
