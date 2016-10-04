#define BOOST_TEST_DYN_LINK

#include "tcp.h"
#include "servers/tcp_server.h"

#include <sstream>
#include <memory>

#include "boost_config.h"
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/future.hpp>

using boost::future;
using boost::asio::buffer;
using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::system::error_code;

using net::Tcp;
using net::Tcp_Server;

using std::ostringstream;
using std::shared_ptr;
using std::string;

static const int port_int = 9043;
static const char* port_str = "9043";

// Test Fixtures
struct A_Running_Tcp_Echo_Server
{
    A_Running_Tcp_Echo_Server() : s(Tcp_Server::Role_t::Echo, port_int)
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
    string send_string = "hello, world!\n";
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

/*
    // XXX is this test case possible? can a tcp connection be checked at anytime?
    BOOST_FIXTURE_TEST_CASE( check_client_closes_properly_when_server_shutsdown,
                             A_Connected_Tcp_Client )
    {
        s.stop();
        BOOST_TEST( !client.is_open() );
    }
*/
    BOOST_FIXTURE_TEST_SUITE( client_sending_messages, A_Connected_Tcp_Client )
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

    BOOST_FIXTURE_TEST_SUITE( client_receiving_messages, A_Connected_Tcp_Client )
            error_code send_ec;
            Tcp::Send_Return_t send_fut;
            size_t sent_size;

            error_code read_ec;
            Tcp::Receive_Return_t recv_fut;
            shared_ptr<string> response;

            BOOST_AUTO_TEST_CASE( check_receive_until_error )
            {
                sent_size = client.send(send_string, send_ec).get();
                BOOST_TEST( !send_ec );

                recv_fut = client.receive(read_ec);
                response = recv_fut.get();

                // No errors
                BOOST_TEST( !read_ec );

                // Receive correct data
                BOOST_TEST( (*response == send_string) );
            }

            BOOST_AUTO_TEST_CASE( check_receive_size )
            {
                sent_size = client.send(send_string, send_ec).get();
                BOOST_TEST( !send_ec );

                size_t receive_size = 5;

                recv_fut = client.receive(receive_size, read_ec);
                response = recv_fut.get();

                // No errors
                BOOST_TEST( !read_ec );

                // Receive correct data
                BOOST_TEST( (*response == send_string.substr(0, receive_size)) );
                // BOOST_TEST( (response->c_str() == "hello") );
                std::cout << "res len = " << response->size() << std::endl;
                std::cout << "substr = " << send_string.substr(0, receive_size) << std::endl;
            }

            BOOST_AUTO_TEST_CASE( check_receive_string_pattern )
            {
                send_fut = client.send(send_string, send_ec);
                sent_size = send_fut.get();
                BOOST_TEST( !send_ec );

                string pattern = ",";
                size_t loc = send_string.find(pattern);

                recv_fut = client.receive(pattern, read_ec);
                response = recv_fut.get();

                // No errors
                BOOST_TEST( !read_ec );

                // Receive correct data
                BOOST_TEST( (*response == send_string.substr(0, loc+1)) );
            }

    BOOST_AUTO_TEST_SUITE_END() // client_receiving_messages

BOOST_AUTO_TEST_SUITE_END()
