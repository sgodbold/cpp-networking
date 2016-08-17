#define BOOST_TEST_DYN_LINK

#include "tcp_pool.h"
#include "servers/tcp_server.h"

#include <memory>

#include <boost/test/unit_test.hpp>

using boost::posix_time::milliseconds;

using net::Tcp;
using net::Tcp_Pool;
using net::Tcp_Server;

using std::shared_ptr;

static const int port_int = 9007;
static const char* port_str = "9007";

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
        auto tg1 = tcp_pool->get();
        auto tg2 = tcp_pool->get();

        BOOST_TEST( (tg1->status() == Tcp::Status_t::Open) );
        BOOST_TEST( (tg2->status() == Tcp::Status_t::Open) );

        // Check independence
        tg1->close();

        BOOST_TEST( (tg1->status() == Tcp::Status_t::Closed) );
        BOOST_TEST( (tg2->status() == Tcp::Status_t::Open) );
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
