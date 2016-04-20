#ifndef CPP_NETWORKING_HTTP
#define CPP_NETWORKING_HTTP

#include "tcp.h"

#include <boost/asio.hpp>
#include <map>
#include <string>

// Settings
const char* protocol_ = "HTTP/1.1";

namespace async_net {

class Http {
public:
    struct Http_Response;

    using Http_Handler = void(*)(boost::system::error_code, Http_Response);

    Http(const std::string& host);

    void request(const std::string& method, const std::string& path,
                 boost::asio::const_buffer& body, Http_Handler h);

    void get(const std::string& path, Http_Handler);

    void post(const std::string& path, boost::asio::const_buffer& body, Http_Handler);

    void head(const std::string& path, Http_Handler);

    void put(const std::string& path, boost::asio::const_buffer& body, Http_Handler);

    void delet(const std::string& path, Http_Handler);

    void trace(const std::string& path, Http_Handler);

    void options(const std::string& path, Http_Handler);

    void connect(const std::string& path, boost::asio::const_buffer& body, Http_Handler);

    void patch(const std::string& path, boost::asio::const_buffer& body, Http_Handler);

    // Setters used to construct a request
    void add_header(const std::string& name, const std::string& value)
        { headers[name] = value; }
    void remove_header(const std::string& name)
        { headers.erase(name); }

    class Http_Response {
        // XXX add helpers
        std::string status;
        std::map<std::string, std::string> headers;
        boost::asio::mutable_buffer body;
    };

private:
    tcp connection;
    const std::string& host;
    std::string protocol;
    std::map<std::string, std::string> headers;

    boost::asio::const_buffer make_request(const std::string& method, const std::string& path);

    void make_response_handle(boost::system::error_code& error, boost::asio::const_buffer);
    Http_Response make_response(boost::asio::const_buffer data);

}; // class Http
} // namespace async_net

#endif
