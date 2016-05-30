#ifndef CPP_NETWORKING_TCP_ECHO_SESSION_H
#define CPP_NETWORKING_TCP_ECHO_SESSION_H

#include "servers/tcp_base_session.h"

#include <vector>

#include <boost/asio.hpp>

namespace net {

class Tcp_Echo_Session : public Tcp_Base_Session
{
public:
    Tcp_Echo_Session(boost::asio::ip::tcp::socket socket);

protected:
    void do_read_work(std::shared_ptr<std::vector<char>> data_ptr,
                      boost::system::error_code) override;

    void do_write_work(boost::system::error_code, std::size_t length) override;
}; // tcp_echo_session

} // net

#endif
