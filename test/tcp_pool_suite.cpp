#define BOOST_TEST_DYN_LINK

#include "logger.h"
#include "tcp_pool.h"
#include "servers/tcp_server.h"

#include <memory>

#include "boost_config.h"
#include <boost/test/unit_test.hpp>

using boost::posix_time::milliseconds;
using boost::system::error_code;

using net::Tcp;
using net::Tcp_Pool;
using net::Tcp_Server;

using std::shared_ptr;

static const int port_int = 9008;
static const char* port_str = "9008";

struct A_Tcp_Pool_Connected_To_Local_Server
{
    A_Tcp_Pool_Connected_To_Local_Server()
        : s(Tcp_Server::Role_t::Passive, port_int),
        tcp_pool(Tcp_Pool::create("localhost", port_str, milliseconds(10)))
    {}

    Tcp_Server s;
    shared_ptr<Tcp_Pool> tcp_pool;
};

BOOST_FIXTURE_TEST_SUITE( tcp_pool_suite, A_Tcp_Pool_Connected_To_Local_Server )

    BOOST_AUTO_TEST_CASE( check_tcp_pool_gets_open_connection )
    {
        Tcp_Pool::Tcp_Guard tcp_guard = tcp_pool->get();

        BOOST_TEST( (tcp_guard->status() == Tcp::Status_t::Open) );
    }

    BOOST_AUTO_TEST_CASE( check_tcp_pool_gets_multiple_independent_open_connections )
    {
        auto tcpg1 = tcp_pool->get();
        auto tcpg2 = tcp_pool->get();

        BOOST_TEST( (tcpg1->status() == Tcp::Status_t::Open) );
        BOOST_TEST( (tcpg2->status() == Tcp::Status_t::Open) );

        // Check independence
        tcpg1->close();

        BOOST_TEST( (tcpg1->status() == Tcp::Status_t::Closed) );
        BOOST_TEST( (tcpg2->status() == Tcp::Status_t::Open) );
    }

    BOOST_AUTO_TEST_CASE( check_multiple_connections_can_independently_send_recv_data )
    {
        auto tcpg1 = tcp_pool->get();
        auto tcpg2 = tcp_pool->get();

        error_code ec1, ec2;

        size_t s1 = tcpg1->send("hello", ec1).get();
        size_t s2 = tcpg2->send("world", ec2).get();

        // Receive in reverse order
        auto recv2 = tcpg2->receive(s2, ec2).get();
        auto recv1 = tcpg1->receive(s1, ec1).get();

        std::ostringstream ss1, ss2;
        ss1 << recv1;
        ss2 << recv2;

        // XXX I'm not sure receive is properly working
        net::Logger::get()->info(ss1.str());
        net::Logger::get()->info(ss1.str());

        BOOST_TEST( ss1.str() == "hello" );
        BOOST_TEST( ss2.str() == "world" );
    }

    // XXX These tests require checking pool internals?

    BOOST_AUTO_TEST_CASE( check_closed_guard_doesnt_return_to_pool )
    {
    }

    BOOST_AUTO_TEST_CASE( check_cached_connection_expires_after_timeout )
    {
    }

    BOOST_AUTO_TEST_CASE( check_tcp_pool_gets_existing_cached_connection )
    {
    }

    BOOST_AUTO_TEST_CASE( check_deleting_connection_belonging_to_deleted_pool )
    {
    }

BOOST_AUTO_TEST_SUITE_END()
