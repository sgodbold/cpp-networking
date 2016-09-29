#include "logger.h"
#include "tcp.h"

#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include "boost_config.h"
#include <boost/asio.hpp>

using boost::asio::buffer; using boost::asio::const_buffer; using boost::asio::streambuf;

using std::cout; using std::cin; using std::cerr; using std::endl;
using std::shared_ptr;
using std::string;

using net::Logger;
using net::Tcp;

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: ./tcp_echo_server <port>" << endl;
            return 1;
        }

        std::string port = std::to_string(std::atoi(argv[1]));
        Tcp client("localhost", port);

        string input;
        boost::system::error_code ec;

        while(true) {
            // Get message to send
            cout << "Send: ";
            cin >> input;
            if(input == "quit") { break; }
            input += "\r\n";

            // Send / receive the message
            auto send_fut = client.send(input, ec);
            size_t sent_size = send_fut.get();
            Logger::get()->info("Sent {} bytes", sent_size);
            auto res_fut = client.receive("\n", ec);

            // Convert response buffer to string
            shared_ptr<streambuf> res_buf = res_fut.get();
            Logger::get()->info("Received {} bytes", res_buf->size());
            std::ostringstream ss;
            ss << res_buf;

            cout << "Recv: " << ss.str();
        }
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
}
