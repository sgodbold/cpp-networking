#include "servers/tcp_session_echo.h"
#include "servers/tcp_session_base.h"

#include <boost/asio.hpp>

#include <iostream>
#include <memory>

using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::system::error_code;

using std::cout; using std::endl;
using std::shared_ptr; using std::make_shared;

Tcp_Session_Echo::Tcp_Session_Echo(boost::asio::ip::tcp::socket socket)
    : Tcp_Session_Base(std::move(socket))
{}

void Tcp_Session_Echo::do_read_work(shared_ptr<streambuf> data_ptr, error_code ec)
{
    cout << " | Working on read data" << endl;
    if (!ec) {
        auto buf = make_shared<const_buffer>(data_ptr->data());
        do_write(buf);
    }
}

void Tcp_Session_Echo::do_write_work(error_code ec, size_t length)
{
    cout << " | Working on write data" << endl;
    if (!ec) {
        do_read();
    }
}
