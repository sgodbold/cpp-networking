#include "servers/tcp_echo_session.h"
#include "servers/tcp_base_session.h"

#include "logger.h"

#include "boost_config.h"
#include <boost/asio.hpp>

#include <memory>
#include <vector>

using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::system::error_code;

using net::Tcp_Echo_Session;

using std::make_shared;
using std::shared_ptr;

const char* const pattern = "\n";

Tcp_Echo_Session::Tcp_Echo_Session(boost::asio::ip::tcp::socket socket)
    : net::Tcp_Base_Session(std::move(socket))
{
    Logger::get()->debug(" | echo session started");
}

void Tcp_Echo_Session::start()
{
  do_read(pattern);
}

void Tcp_Echo_Session::do_read_work(shared_ptr<streambuf> res, error_code ec)
{
    Logger::get()->debug(" | working on read data");
    if (!ec)
    {
        do_write(res);
    }
}

void Tcp_Echo_Session::do_write_work(error_code ec, size_t length)
{
    Logger::get()->debug(" | working on write data");
    if (!ec)
    {
        do_read(pattern);
    }
}
