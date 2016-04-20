#ifndef CPP_NETWORKING_HTTP
#define CPP_NETWORKING_HTTP

#include "tcp.h"

#include <boost/asio.hpp>
#include <map>
#include <string>

// Settings
const char* protocol_ = "HTTP/1.1";

namespace net {

class http {
public:
    struct http_response;

    http(const std::string& host);

    http_response request(const std::string& method, const std::string& path,
                          boost::asio::const_buffer& body);
    http_response request(const std::string& method, const std::string& path);

    void get(const std::string& path);
    void post(const std::string& path, boost::asio::const_buffer& body);
    /* XXX
     * head()
     * put()
     * delete()
     * trace()
     * options()
     * connect()
     * patch()
     */

    // Setters used to construct a request
    void add_header(const std::string& name, const std::string& value)
        { headers[name] = value; }
    void remove_header(const std::string& name)
        { headers.erase(name); }

    struct http_response {
        std::string status;
        std::vector<std::pair<std::string, std::string>> headers;
        boost::asio::mutable_buffer body;
    };

private:
    tcp connection;
    const std::string& host;
    std::string protocol;
    std::map<std::string, std::string> headers;

    boost::asio::const_buffer make_request(const std::string& method, const std::string& path);

}; // http
} // net

#endif
