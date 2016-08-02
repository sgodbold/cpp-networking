#include <catch.hpp>

#include "servers/tcp_server.h"
#include "tcp.h"

#include <memory>

using net::Tcp;
using net::Tcp_Server;

using std::make_shared;
using std::shared_ptr;

static const int port_int = 9004;
static const char* port_str = "9004";

TEST_CASE("construct and destroy a TCP Echo Server", "[tcp_server][echo]")
{
    shared_ptr<Tcp_Server> s;
    REQUIRE_NOTHROW( s = make_shared<Tcp_Server>(Tcp_Server::Role_t::Echo, port_int) );
}

SCENARIO("Running a TCP Echo Server", "[tcp_server][echo]")
{
    GIVEN("a running echo server")
    {
        shared_ptr<Tcp_Server> s;
        REQUIRE_NOTHROW( s = make_shared<Tcp_Server>(Tcp_Server::Role_t::Echo, port_int) );

        WHEN("a tcp client connects")
        {
            Tcp client("localhost", port_str);
            
            THEN("the client has an open connection")
            {
                CHECK(client.status() == Tcp::Status_t::Open);
            }
        }
    }
}

SCENARIO("Running a TCP Passive Server", "[tcp_server][passive]")
{
    GIVEN("a running passive server")
    {
        shared_ptr<Tcp_Server> s;
        REQUIRE_NOTHROW( s = make_shared<Tcp_Server>(Tcp_Server::Role_t::Passive, port_int) );

        WHEN("a tcp client connects")
        {
            Tcp client("localhost", port_str);

            THEN("the client has an open connection")
            {
                CHECK(client.status() == Tcp::Status_t::Open);
            }

            /* TODO
            AND_WHEN("the server is destroyed")
            {
                s.reset();

                THEN("the client becomes cleanly disconnected")
                {
                    CHECK(client.status() == Tcp::Status_t::Closed);
                }
            }
            */
        }
    }
}
