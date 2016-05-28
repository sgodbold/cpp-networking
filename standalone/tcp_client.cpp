#include "tcp.h"

#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include <boost/asio.hpp>

using boost::asio::buffer; using boost::asio::const_buffer; using boost::asio::streambuf;

using std::cout; using std::cin; using std::cerr; using std::endl;
using std::shared_ptr;
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
            // Get message to send
            cout << "Send: ";
            cin >> input;
            if(input == "quit") { break; }
            input += "\r\n";

            // Send / receive the message
            const_buffer send_buf(buffer(input));
            client.send(send_buf, ec)->get();
            auto res_fut = client.receive_line(ec);

            // Convert response buffer to string
            shared_ptr<streambuf> res_buf = res_fut.get();
            std::ostringstream ss;
            ss << res_buf;

            cout << "Recv: " << ss.str();
        }
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
}
