#define BOOST_TEST_DYN_LINK

#include "logger.h"
#include "tcp_pool.h"
#include "servers/tcp_server.h"

#include <memory>
#include <string>

#include "boost_config.h"
#include <boost/test/unit_test.hpp>

using boost::posix_time::milliseconds;
using boost::system::error_code;

using net::Tcp;
using net::Tcp_Pool;
using net::Tcp_Server;

using std::shared_ptr;
using std::string;

static const int port_int = 9008;
static const char* port_str = "9008";

struct A_Tcp_Pool_Connected_To_Local_Server
{
    A_Tcp_Pool_Connected_To_Local_Server()
        : s(Tcp_Server::Role_t::Echo, port_int),
        tcp_pool(Tcp_Pool::create("localhost", port_str, milliseconds(10)))
    {}

    Tcp_Server s;
    shared_ptr<Tcp_Pool> tcp_pool;
};

BOOST_FIXTURE_TEST_SUITE( tcp_pool_suite, A_Tcp_Pool_Connected_To_Local_Server )

    BOOST_AUTO_TEST_CASE( check_tcp_pool_gets_open_connection )
    {
        Tcp_Pool::Tcp_Guard tcp_guard = tcp_pool->get();
        BOOST_TEST( tcp_guard->is_open() );
    }

    BOOST_AUTO_TEST_CASE( check_tcp_pool_gets_multiple_independent_open_connections )
    {
        auto tcpg1 = tcp_pool->get();
        auto tcpg2 = tcp_pool->get();

        BOOST_TEST( tcpg1->is_open() );
        BOOST_TEST( tcpg2->is_open() );

        // Check independence
        tcpg1->close();

        BOOST_TEST( !tcpg1->is_open() );
        BOOST_TEST( tcpg2->is_open() );
    }

    BOOST_AUTO_TEST_CASE( check_guard_can_send_and_receive )
    {
        auto tcpg = tcp_pool->get();

        BOOST_TEST( tcpg->is_open() );

        std::string send_data = "hello, world!\n";
        error_code ec;

        size_t s1 = tcpg->send(send_data, ec).get();

        BOOST_TEST( !ec );
        BOOST_TEST( s1 == send_data.size() );

        auto recv = tcpg->receive(s1, ec).get();

        BOOST_TEST( !ec );
        BOOST_TEST( *recv == send_data );
    }

    BOOST_AUTO_TEST_CASE( check_multiple_connections_can_independently_send_recv_data )
    {
        auto tcpg1 = tcp_pool->get();
        auto tcpg2 = tcp_pool->get();
        const string str1 = "hello\n";
        const string str2 = "world\n";

        BOOST_TEST( tcpg1->is_open() );
        BOOST_TEST( tcpg2->is_open() );

        error_code ec1, ec2;
        size_t s1 = tcpg1->send(str1, ec1).get();
        size_t s2 = tcpg2->send(str2, ec2).get();

        BOOST_TEST( !ec1 );
        BOOST_TEST( !ec2 );
        BOOST_TEST( s1 == str1.size() );
        BOOST_TEST( s2 == str2.size() );
        BOOST_TEST( tcpg1->is_open() );
        BOOST_TEST( tcpg2->is_open() );

        // Receive in reverse order
        auto recv2 = tcpg2->receive(s2, ec2).get();
        auto recv1 = tcpg1->receive(s1, ec1).get();

        BOOST_TEST( *recv1 == str1 );
        BOOST_TEST( *recv2 == str2 );
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
