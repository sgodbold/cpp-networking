#include "servers/tcp_base_session.h"

#include <iostream>
#include <memory>
#include <vector>

#include "boost_config.h"
#include <boost/asio.hpp>

using boost::asio::buffer;
using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::system::error_code;

using net::Tcp_Base_Session;

using std::shared_ptr;
using std::make_shared;

Tcp_Base_Session::Tcp_Base_Session(boost::asio::ip::tcp::socket socket_)
    : socket(std::move(socket_))
{}

Tcp_Base_Session::~Tcp_Base_Session() {}

void Tcp_Base_Session::start()
{
    do_read();
}

void Tcp_Base_Session::do_read()
{
    // XXX i think this is needed to make sure this object lives as long as this function
    // needs to run. Bad things would happen if all other pointers are deleted while this
    // is running.
    auto self(shared_from_this());

    // XXX replace with streambufs
    auto data_ptr = make_shared<std::vector<char>>(max_length_c);

    std::cout << " | Waiting to read..." << std::endl;
    socket.async_read_some(boost::asio::buffer(*data_ptr, max_length_c),
        [this, self, data_ptr] (error_code ec, std::size_t length) {
            // XXX Check ec for client disconnection
            std::cout << " | Read " << length << " bytes" << std::endl;
            this->do_read_work(data_ptr, ec);
        }
    );
}

void Tcp_Base_Session::do_write(shared_ptr<const_buffer> data)
{
    auto self(shared_from_this());

    async_write(socket, buffer(*data, max_length_c),
        [this, self] (error_code ec, std::size_t length) {
            // XXX Check ec for client disconnection
            std::cout << " | Wrote " << length << " bytes" << std::endl;
            this->do_write_work(ec, length);
        }
    );
}
