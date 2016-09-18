#include "tcp_pool.h"

#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include "boost_config.h"
#include <boost/asio.hpp>

using boost::asio::buffer;
using boost::asio::const_buffer;
using boost::asio::streambuf;
using boost::posix_time::milliseconds;
using boost::posix_time::seconds;

using std::cout; using std::cin; using std::cerr; using std::endl;
using std::shared_ptr;
using std::string;

using net::Tcp_Pool;

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: ./tcp_echo_server <port>" << endl;
            return 1;
        }

        std::string port = std::to_string(std::atoi(argv[1]));
        string input;
        boost::system::error_code ec;

        auto pool = Tcp_Pool::create("localhost", port, seconds(10));

        while(true) {
            Tcp_Pool::Tcp_Guard client = pool->get();

            // Get message to send
            cout << "Send: ";
            cin >> input;
            if(input == "quit") { break; }
            input += "\r\n";

            // Send / receive the message
            const_buffer send_buf(buffer(input));
            auto send_fut = client->send(send_buf, ec);
            cout << "Sent " << send_fut.get() << " bytes" << endl;
            auto res_fut = client->receive("\n", ec);

            // Convert response buffer to string
            shared_ptr<streambuf> res_buf = res_fut.get();
            cout << "Received " << res_buf->size() << " bytes" << endl;
            std::ostringstream ss;
            ss << res_buf;

            // Close the connection
            // client->close();

            cout << "Recv: " << ss.str();
        }
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
}
