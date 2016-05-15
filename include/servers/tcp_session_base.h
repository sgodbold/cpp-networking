#ifndef CPP_NETWORKING_TCP_SESSION_BASE_H
#define CPP_NETWORKING_TCP_SESSION_BASE_H

#include <memory>

#include <boost/asio.hpp>

class Tcp_Session_Base : public std::enable_shared_from_this<Tcp_Session_Base>
{
public:
    Tcp_Session_Base(boost::asio::ip::tcp::socket socket_);
    virtual ~Tcp_Session_Base() = 0;

    virtual void start();

protected:
    void do_read();
    void do_write(std::shared_ptr<boost::asio::const_buffer> data);

    // Called by do_read and do_write. Overload to specify what happens
    // after the read / write operation is done.
    virtual void do_read_work(std::shared_ptr<boost::asio::streambuf> data_ptr,
                              boost::system::error_code) = 0;
    virtual void do_write_work(boost::system::error_code, std::size_t length) = 0;

private:
    boost::asio::ip::tcp::socket socket;
};

#endif