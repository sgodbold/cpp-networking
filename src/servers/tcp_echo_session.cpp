#include "servers/tcp_echo_session.h"
#include "servers/tcp_base_session.h"

#include "logger.h"

#include "boost_config.h"
#include <boost/asio.hpp>

#include <memory>
#include <vector>

using boost::asio::const_buffer;
using boost::system::error_code;

using net::Tcp_Echo_Session;

using std::make_shared;
using std::shared_ptr;

Tcp_Echo_Session::Tcp_Echo_Session(boost::asio::ip::tcp::socket socket)
    : net::Tcp_Base_Session(std::move(socket))
{
    Logger::get()->debug(" | echo session started");
}

void Tcp_Echo_Session::do_read_work(shared_ptr<std::vector<char>> data_ptr, error_code ec)
{
    Logger::get()->debug(" | working on read data");
    if (!ec)
    {
        auto buf = make_shared<const_buffer>(boost::asio::buffer(*data_ptr, max_length_c));
        do_write(buf);
    }
}

void Tcp_Echo_Session::do_write_work(error_code ec, size_t length)
{
    Logger::get()->debug(" | working on write data");
    if (!ec)
    {
        do_read();
    }
}
