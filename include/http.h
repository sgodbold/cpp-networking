#ifndef CPP_NETWORKING_HTTP
#define CPP_NETWORKING_HTTP

#include "tcp.h"

#include <boost/asio.hpp>
#include <map>
#include <string>

// Settings
const char* http_version_ = "HTTP/1.1";

namespace async_net {

class Http {
public:
    struct Http_Response;

    using Http_Handler_t = std::function<void(boost::system::error_code, Http_Response)>;
    using Headers_t = std::map<std::string, std::string>;

    explicit Http(const std::string& host);

    void request(const std::string& method, const std::string& path,
                 boost::asio::const_buffer& body, Http_Handler_t);

    void get(const std::string& path, Http_Handler_t);

    void post(const std::string& path, boost::asio::const_buffer& body, Http_Handler_t);

    void head(const std::string& path, Http_Handler_t);

    void put(const std::string& path, boost::asio::const_buffer& body, Http_Handler_t);

    void delet(const std::string& path, Http_Handler_t);

    void trace(const std::string& path, Http_Handler_t);

    void options(const std::string& path, Http_Handler_t);

    void connect(const std::string& path, boost::asio::const_buffer& body, Http_Handler_t);

    void patch(const std::string& path, boost::asio::const_buffer& body, Http_Handler_t);

    // Setters used to construct a request
    void add_header(const std::string& name, const std::string& value)
        { headers[name] = value; }
    void remove_header(const std::string& name)
        { headers.erase(name); }

    struct Http_Response {
        std::string status;
        std::map<std::string, std::string> headers;
        boost::asio::const_buffer body;
    };

private:
    Tcp connection;
    const std::string& host;
    std::string http_version;
    Headers_t headers;

    boost::asio::const_buffer make_request(const std::string& method, const std::string& path);

    Http_Response make_response(boost::asio::const_buffer data);

}; // class Http
} // namespace async_net

#endif
