#ifndef CPP_NETWORKING_TCP_SESSION_BASE_H
#define CPP_NETWORKING_TCP_SESSION_BASE_H

#include <memory>

#include "boost_config.h"
#include <boost/asio.hpp>

namespace net {

class Tcp_Base_Session : public std::enable_shared_from_this<Tcp_Base_Session>
{
    public:
        Tcp_Base_Session(boost::asio::ip::tcp::socket socket_);
        virtual ~Tcp_Base_Session() = 0;

        virtual void start() = 0;

    protected:
        // Receive until a certain pattern is reached.
        void do_read(const std::string& s);

        // Read until a certain size is reached.
        void do_read(size_t n);

        void do_write(std::shared_ptr<boost::asio::streambuf> data);

        // Called by do_read and do_write. Overload to specify what happens
        // after the read / write operation is done.
        virtual void do_read_work(std::shared_ptr<boost::asio::streambuf>,
                                  boost::system::error_code) = 0;
        virtual void do_write_work(boost::system::error_code, std::size_t length) = 0;

    private:
        boost::asio::ip::tcp::socket socket;

}; // Tcp_Base_Session

} // net

#endif
