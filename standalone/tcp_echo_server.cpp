#include "servers/tcp_server.h"

#include <iostream>

#include <boost/asio.hpp>

using std::cout; using std::cerr; using std::endl;

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: ./tcp_echo_server <port>" << endl;
            return 1;
        }

        boost::asio::io_service io_service;

        Tcp_Server s(io_service, std::atoi(argv[1]));

        io_service.run();
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
}
