#include "servers/tcp_echo_session.h"
#include "servers/tcp_server.h"

#include <iostream>

#include <boost/asio.hpp>

using std::cout; using std::cerr; using std::endl;
using net::Tcp_Server;

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: ./tcp_echo_server <port>" << endl;
            return 1;
        }

        net::Tcp_Server s(net::Tcp_Server::Role_t::Echo, std::atoi(argv[1]));

        while(true);
    }
    catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
}
