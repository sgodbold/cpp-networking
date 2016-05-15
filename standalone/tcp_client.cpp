#include "tcp.h"

#include <iostream>
#include <string>

#include <boost/asio.hpp>

using std::cout; using std::cin; using std::cerr; using std::endl;
using std::string;

using net::Tcp;

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: ./tcp_echo_server <port>" << endl;
            return 1;
        }

        std::string port = std::to_string(std::atoi(argv[1]));
        string input;
        boost::system::error_code ec;

        Tcp client("localhost", port);

        while(true) {
            cout << "Send: ";
            cin >> input;

            boost::asio::const_buffer send_buf(boost::asio::buffer(input));
            auto len = client.send(send_buf, ec);
            cout << "Waiting for send..." << endl;
            cout << len->get() << endl;

            auto res_fut = client.receive(ec);
            cout << "Waiting for receive..." << endl;
            boost::asio::const_buffer res_buf = res_fut->get();

            std::size_t s1 = boost::asio::buffer_size(res_buf);
            const unsigned char* p1 = boost::asio::buffer_cast<const unsigned char*>(res_buf);

            cout << "Recv: " << p1 << endl;
        }
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
}
