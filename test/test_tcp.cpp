#include "tcp.h"
#include "servers/tcp_server.h"

#include <catch.hpp>
#include <string>

#include "boost_definitions.h"
#include <boost/asio.hpp>

using boost::asio::const_buffer;
using boost::asio::buffer;

using net::Tcp;
using net::Tcp_Server;

using std::string;

const int port_int = 9002;
const char* port_str = "9002";

// Tcp_Server s(Tcp_Server::Role_t::Echo, port_int);

SCENARIO("TCP Client Connecting and Disconnecting", "[tcp][client]")
{
    GIVEN("a local echo server")
    {
        Tcp_Server s(Tcp_Server::Role_t::Echo, port_int);

        WHEN("a connection is opened")
        {
            Tcp client("localhost", port_str);

            THEN("it connects with no errors")
            {
                REQUIRE(client.status() == Tcp::Status_t::Open);
            }

            AND_WHEN("a connection is closed")
            {
                client.close();

                THEN("it disconnects with no errors")
                {
                    REQUIRE(client.status() == Tcp::Status_t::Closed);
                }
            }
        }
    }
}

/*
SCENARIO("TCP Client Communicating", "[tcp]")
{
    GIVEN("a local echo server")
    {
        static Tcp_Server s(Tcp_Server::Role_t::Echo, port_int);
    }
}
*/
