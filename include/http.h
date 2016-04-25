#ifndef CPP_NETWORKING_HTTP
#define CPP_NETWORKING_HTTP

#include "tcp.h"

#include <map>
#include <string>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

// Settings
const char* http_version_ = "HTTP/1.1";

namespace net {

class Http {
public:
    struct Http_Response;

    using Http_Handler_t = std::function<void(boost::system::error_code, Http_Response)>;
    using Headers_t = std::map<std::string, std::string>;

    explicit Http(const std::string& host);

    boost::future<Http_Response> request(const std::string& method, const std::string& path,
                                         boost::asio::const_buffer& body);

    boost::future<Http_Response> get(const std::string& path);

    boost::future<Http_Response> post(const std::string& path, boost::asio::const_buffer& body);

    boost::future<Http_Response> head(const std::string& path);

    boost::future<Http_Response> put(const std::string& path, boost::asio::const_buffer& body);

    boost::future<Http_Response> delet(const std::string& path);

    boost::future<Http_Response> trace(const std::string& path);

    boost::future<Http_Response> options(const std::string& path);

    boost::future<Http_Response> connect(const std::string& path, boost::asio::const_buffer& body);

    boost::future<Http_Response> patch(const std::string& path, boost::asio::const_buffer& body);

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

    boost::future<Http_Response> promise_response(std::vector<boost::asio::const_buffer>& req);

}; // class Http
} // namespace net

#endif
