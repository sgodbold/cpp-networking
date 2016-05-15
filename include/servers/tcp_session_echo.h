#ifndef CPP_NETWORKING_TCP_SESSION_ECHO_H
#define CPP_NETWORKING_TCP_SESSION_ECHO_H

#include "servers/tcp_session_base.h"

#include <vector>

#include <boost/asio.hpp>

class Tcp_Session_Echo : public Tcp_Session_Base
{
public:
    Tcp_Session_Echo(boost::asio::ip::tcp::socket socket);

protected:
    void do_read_work(std::shared_ptr<std::vector<char>> data_ptr,
                      boost::system::error_code) override;

    void do_write_work(boost::system::error_code, std::size_t length) override;
};

#endif
