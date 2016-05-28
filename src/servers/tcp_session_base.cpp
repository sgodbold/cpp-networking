#include "servers/tcp_session_base.h"

#include <iostream>
#include <memory>
#include <vector>

#include <boost/asio.hpp>

using boost::asio::buffer;
using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::system::error_code;

using std::shared_ptr;
using std::make_shared;

Tcp_Session_Base::Tcp_Session_Base(boost::asio::ip::tcp::socket socket_)
    : socket(std::move(socket_))
{
    std::cout << " | New tcp session" << std::endl;
}

Tcp_Session_Base::~Tcp_Session_Base() {}

void Tcp_Session_Base::start()
{
    do_read();
}

void Tcp_Session_Base::do_read()
{
    // XXX i think this is needed to make sure this object lives as long as this function
    // needs to run. Bad things would happen if all other pointers are deleted while this
    // is running.
    auto self(shared_from_this());
    // auto data_ptr = make_shared<streambuf>();
    auto data_ptr = make_shared<std::vector<char>>(max_length_c);

    std::cout << " | Waiting to read..." << std::endl;
    // async_read(socket, *data_ptr, [this, self, data_ptr] (error_code ec, std::size_t length) {
    socket.async_read_some(boost::asio::buffer(*data_ptr, max_length_c),
            [this, self, data_ptr] (error_code ec, std::size_t length) {
                std::cout << " | Read " << length << " bytes" << std::endl;
                this->do_read_work(data_ptr, ec);
            }
    );
}

void Tcp_Session_Base::do_write(shared_ptr<const_buffer> data)
{
    auto self(shared_from_this());

    async_write(socket, buffer(*data, max_length_c), [this, self] (error_code ec, std::size_t length) {
        std::cout << " | Wrote " << length << " bytes" << std::endl;
        this->do_write_work(ec, length);
    });
}
