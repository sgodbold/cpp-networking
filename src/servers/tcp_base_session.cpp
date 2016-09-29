#include "servers/tcp_base_session.h"

#include "logger.h"

#include <memory>

#include "boost_config.h"
#include <boost/asio.hpp>

using boost::asio::async_read;
using boost::asio::async_read_until;
using boost::asio::streambuf;
using boost::system::error_code;

using net::Tcp_Base_Session;

using std::make_shared;
using std::shared_ptr;
using std::string;

Tcp_Base_Session::Tcp_Base_Session(boost::asio::ip::tcp::socket socket_)
    : socket(std::move(socket_))
{}

Tcp_Base_Session::~Tcp_Base_Session() {}

void Tcp_Base_Session::do_read(const string& pattern)
{
    auto self(shared_from_this());
    auto res = make_shared<streambuf>();
    async_read_until(socket, *res, pattern, [this, self, res]  (const error_code& ec, size_t length)
        {
            Logger::get()->debug(" | read {} bytes", length);
            this->do_read_work(res, ec);
        }
    );
}

void Tcp_Base_Session::do_read(size_t len)
{
    auto self(shared_from_this());
    auto res = make_shared<streambuf>();
    async_read(socket, *res, boost::asio::transfer_at_least(len), [this, self, res]
               (const error_code& ec, size_t length)
        {
            Logger::get()->debug(" | read {} bytes", length);
            this->do_read_work(res, ec);
        }
    );
}

void Tcp_Base_Session::do_write(shared_ptr<streambuf> buf)
{
    auto self(shared_from_this());

    async_write(socket, *buf,
        [this, self] (error_code ec, std::size_t length) {
            // XXX Check ec for client disconnection
            Logger::get()->debug(" | wrote {} bytes", length);
            this->do_write_work(ec, length);
        }
    );
}
