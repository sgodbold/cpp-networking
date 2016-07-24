#include <catch.hpp>

#include "servers/tcp_server.h"

using net::Tcp_Server;

TEST_CASE("construct and destroy a TCP Echo Server", "[tcp][server]")
{
    Tcp_Server* s;
    REQUIRE_NOTHROW( s = new Tcp_Server(Tcp_Server::Role_t::Passive, 9005) );
    REQUIRE_NOTHROW( delete s );
}
