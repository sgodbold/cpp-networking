#define BOOST_TEST_DYN_LINK

#include "servers/tcp_server.h"
#include "tcp.h"

#include "boost_config.h"
#include <boost/test/unit_test.hpp>

#include <memory>

using net::Tcp;
using net::Tcp_Server;

using std::make_shared;
using std::shared_ptr;

static const int port_int = 9005;
static const char* port_str = "9005";

struct A_Running_Echo_Server
{
    A_Running_Echo_Server() : s(Tcp_Server::Role_t::Echo, port_int) {}

    Tcp_Server s;
};

BOOST_AUTO_TEST_SUITE( tcp_server_suite )

    BOOST_FIXTURE_TEST_SUITE( echo_role, A_Running_Echo_Server )

        BOOST_AUTO_TEST_CASE( check_client_connection )
        {
            Tcp client("localhost", port_str);

            BOOST_TEST( client.is_open() );
        }

        BOOST_AUTO_TEST_CASE( check_client_is_disconnected_when_server_stops )
        {
            // TODO
        }

        // TODO: check server internals when messages are recieved / sent ?
        // TODO: check accept loop?

    BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
