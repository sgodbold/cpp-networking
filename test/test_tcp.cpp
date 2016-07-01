#include "tcp.h"
#include "servers/tcp_server.h"

#include <catch.hpp>
#include <memory>
#include <string>

#include "boost_definitions.h"
#include <boost/asio.hpp>

using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::asio::buffer;

using net::Tcp;
using net::Tcp_Server;

using std::string;

const int port_int = 9002;
const char* port_str = "9002";

string get_receive_message(std::shared_ptr<streambuf>& recv_buf);

SCENARIO("TCP Client Connecting and Disconnecting", "[tcp][client]")
{
    GIVEN("a local echo server")
    {
        Tcp_Server s(Tcp_Server::Role_t::Passive, port_int);

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

SCENARIO("TCP Client Sending Messages", "[tcp][client][send]")
{
    GIVEN("a local echo server and connection")
    {
        Tcp_Server s(Tcp_Server::Role_t::Echo, port_int);
        Tcp client("localhost", port_str);

        // Single buffers to send
        string send_message = "hello, world!";
        const_buffer send_buf(buffer(send_message));

        // Vector of buffers to send
        int num_send_buffers = 3;
        std::vector<const_buffer> send_buffers;
        for (int i=0; i < num_send_buffers; ++i) {
            send_buffers.push_back(buffer(send_message));
        }

        WHEN("a const buffer is sent")
        {
            boost::system::error_code ec;
            auto send_fut = client.send(send_buf, ec);
            
            THEN("there are no errors")
            {
                REQUIRE(!ec);
            }

            THEN("the correct length was sent")
            {
                CHECK(send_fut.get() == send_message.size());
            }
        }

        WHEN("a vector of const buffers is sent")
        {
            boost::system::error_code ec;
            auto send_fut = client.send(send_buffers, ec);

            THEN("there are no errors")
            {
                REQUIRE(!ec);
            }

            THEN("the correct length was sent")
            {
                CHECK(send_fut.get() == num_send_buffers * send_message.size());
            }
        }
    }
}

SCENARIO("TCP Client Receiving Messages", "[tcp][client][receive]")
{
    GIVEN("a local echo server and connection")
    {
        Tcp_Server s(Tcp_Server::Role_t::Echo, port_int);
        Tcp client("localhost", port_str);

        string send_message = "hello, world!\n";
        const_buffer send_buf(buffer(send_message));

        boost::system::error_code ec;
        auto send_fut = client.send(send_buf, ec);

        REQUIRE(!ec);

        WHEN("receive a message of a specific size")
        {
            // Receive 'hello' from the server
            auto recv_fut = client.receive(5, ec);
            auto recv_buf = recv_fut.get();
            string expected = "hello";

            THEN("there are no errors")
            {
                REQUIRE(!ec);
            }

            THEN("the correct message was received")
            {
                string received_message = get_receive_message(recv_buf);
                CAPTURE(received_message);
                CHECK(received_message == expected);
            }
        }

        // XXX failing. probably because of the server
        WHEN("a lined message is received")
        {
            auto recv_fut = client.receive_line(ec);
            auto recv_buf = recv_fut.get();

            THEN("there are no errors")
            {
                REQUIRE(!ec);
            }

            THEN("the correct length was received")
            {
                CHECK(recv_buf->size() == send_message.size());
            }

            THEN("the correct message was received")
            {
                string received_message = get_receive_message(recv_buf);
                CAPTURE(received_message);
                CHECK(received_message == send_message);
            }
        }
    }
}

string get_receive_message(std::shared_ptr<streambuf>& recv_buf)
{
    std::ostringstream ss;
    ss << recv_buf;
    return ss.str();
}
