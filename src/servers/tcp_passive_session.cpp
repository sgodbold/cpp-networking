#include "servers/tcp_passive_session.h"
#include "servers/tcp_base_session.h"

#include "boost_config.h"
#include <boost/asio.hpp>

#include <iostream>
#include <memory>
#include <vector>

using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::system::error_code;

using net::Tcp_Passive_Session;

using std::cout;
using std::endl;
using std::make_shared;
using std::shared_ptr;

Tcp_Passive_Session::Tcp_Passive_Session(boost::asio::ip::tcp::socket socket)
    : net::Tcp_Base_Session(std::move(socket))
{
    cout << " | Passive session started" << endl;
}

void Tcp_Passive_Session::do_read_work(shared_ptr<std::vector<char>>, error_code)
{}

void Tcp_Passive_Session::do_write_work(error_code, size_t)
{}
