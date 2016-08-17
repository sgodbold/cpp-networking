#define BOOST_TEST_DYN_LINK

#include "tcp.h"
#include "servers/tcp_server.h"

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/future.hpp>

using boost::asio::buffer;
using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::future;
using boost::system::error_code;

using net::Tcp;
using net::Tcp_Server;

using std::string;

static const int port_int = 9023;
static const char* port_str = "9023";

struct A_Running_Tcp_Echo_Server
{
    A_Running_Tcp_Echo_Server() : s(Tcp_Server::Role_t::Passive, port_int)
    {}

    Tcp_Server s;
};

struct A_Connected_Tcp_Client : public A_Running_Tcp_Echo_Server
{
    A_Connected_Tcp_Client()
        : A_Running_Tcp_Echo_Server(), client("localhost", port_str),
        send_buffer(buffer(send_string)), send_vector_of_buffers(3, send_buffer)
    {}
    
    Tcp client;

    // Sendable Data
    string send_string = "hello, world!";
    const_buffer send_buffer;
    int send_number = 15;
    std::vector<const_buffer> send_vector_of_buffers;
};

BOOST_AUTO_TEST_SUITE( tcp_suite )

    BOOST_FIXTURE_TEST_CASE( check_client_can_connect, A_Connected_Tcp_Client )
    {
        BOOST_TEST( (client.status() == Tcp::Status_t::Open) );
    }

    BOOST_FIXTURE_TEST_CASE( check_client_can_disconnect, A_Connected_Tcp_Client )
    {
        client.close();

        BOOST_TEST( (client.status() == Tcp::Status_t::Closed) );
    }

// XXX re-enable once tcp destructor works when the server is already closed
    BOOST_FIXTURE_TEST_SUITE( client_sending_messages, A_Connected_Tcp_Client,
                              * boost::unit_test::disabled() )

        error_code ec;
        Tcp::Send_Return_t send_fut;
        size_t sent_size;

        BOOST_AUTO_TEST_CASE( check_send_const_buffer )
        {
            send_fut = client.send(send_buffer, ec);
            sent_size = send_fut.get();

            // No errors
            BOOST_TEST( !ec );

            // Sent correct size
            BOOST_TEST( (sent_size == send_string.size()) );
        }

        BOOST_AUTO_TEST_CASE( check_send_vector_of_const_buffers )
        {
            send_fut = client.send(send_vector_of_buffers, ec);
            sent_size = send_fut.get();

            // No errors
            BOOST_TEST( !ec );

            // Sent correct size
            BOOST_TEST( (sent_size == send_string.size() * send_vector_of_buffers.size()) );
        }

        BOOST_AUTO_TEST_CASE( check_send_string )
        {
            send_fut = client.send(send_string, ec);
            sent_size = send_fut.get();

            // No errors
            BOOST_TEST( !ec );

            // Sent correct size
            BOOST_TEST( (sent_size == send_string.size()) );
        }

        BOOST_AUTO_TEST_CASE( check_send_int )
        {
            send_fut = client.send(send_number, ec);
            sent_size = send_fut.get();

            // No errors
            BOOST_TEST( !ec );

            // Sent correct size
            BOOST_TEST( (sent_size == std::to_string(send_number).size()) );
        }

    BOOST_AUTO_TEST_SUITE_END() // client_sending_messages

    BOOST_AUTO_TEST_SUITE( client_receiving_messages )
        // TODO: refactor tcp servers first
    BOOST_AUTO_TEST_SUITE_END() // client_receiving_messages

BOOST_AUTO_TEST_SUITE_END()
