#include "tcp_pool.h"
#include "servers/tcp_server.h"

#include <catch.hpp>

using boost::posix_time::milliseconds;

using net::Tcp;
using net::Tcp_Pool;
using net::Tcp_Server;

static const int port_int = 9007;
static const char* port_str = "9007";

SCENARIO("Using a Tcp Pool", "[tcp_pool]")
{
    auto tcp_pool = Tcp_Pool::create("localhost", port_str, milliseconds(10));

    GIVEN("a local passive tcp server")
    {
        Tcp_Server s(Tcp_Server::Role_t::Echo, port_int);

        WHEN("we get a connection from the pool")
        {
            Tcp_Pool::Tcp_Guard tcp_guard = tcp_pool->get();

            THEN("the received connection is open")
            {
                CHECK(tcp_guard->status() == Tcp::Status_t::Open);
            }
        }
    }
}
