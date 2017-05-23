#ifndef CPP_NETWORKING_HTTP_H
#define CPP_NETWORKING_HTTP_H

#include "tcp.h"
#include "tcp_pool.h"

#include <map>
#include <memory>
#include <string>

#include "boost_config.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

/* Http
 *
 * Overview:
 * This class manages formatting and sending data, parsing responses, and handling automatic
 * behavior expected of an HTTP client.
 *
 * Behavior:
 *
 * Design:
 * - The main interfaces allow for customizing headers and making requests for different HTTP
 * methods. The core of the client uses Tcp_Pool to allow for HTTP1.1 style connection
 * caching. Every request on this client uses its own connection to 'host' to ensure
 * requests and responses are paired together.
 * - Request interfaces assume that the caller will preserve sent data until response
 * is received.
 * - Header interfaces copy values into the client.
 *
 */

// Settings
// XXX should probably be moved to a file and read in
const char* http_version_c = "HTTP/1.1";
const char* http_service_c = "http";
const char* https_service_c = "https";
const boost::posix_time::time_duration default_timeout_c = boost::posix_time::seconds(240);

namespace net {

struct Http_Response
{
    std::string status;
    std::map<std::string, std::string> headers;
    boost::asio::const_buffer body;
};

class Http
{
    public:
        // XXX blocking
        explicit Http(const std::string& host);

        // XXX blocking
        explicit Http(const std::string& host, const std::string& service);

        // Caller must
        boost::future<Http_Response> get(const std::string& path);

        boost::future<Http_Response> post(const std::string& path, boost::asio::const_buffer& body);

        boost::future<Http_Response> post(const std::string& path, const std::string& body);

        boost::future<Http_Response> head(const std::string& path);

        boost::future<Http_Response> put(const std::string& path, boost::asio::const_buffer& body);

        boost::future<Http_Response> put(const std::string& path, const std::string& body);

        boost::future<Http_Response> delet(const std::string& path);

        boost::future<Http_Response> trace(const std::string& path);

        boost::future<Http_Response> options(const std::string& path);

        boost::future<Http_Response> connect(const std::string& path, boost::asio::const_buffer& body);
        boost::future<Http_Response> connect(const std::string& path, const std::string& body);

        boost::future<Http_Response> patch(const std::string& path, boost::asio::const_buffer& body);

        boost::future<Http_Response> patch(const std::string& path, const std::string& body);

        // Header values are copied into the HTTP client so the caller doesn't need to
        // worry about preserving them
        void add_header(const std::string& name, const std::string& value)
            { headers[name] = value; }
        void remove_header(const std::string& name)
            { headers.erase(name); }
        void reset_headers()
            { headers.clear(); }

    private:
        const std::string& host;
        const std::string& service;
        const std::string& http_version;

        std::shared_ptr<Tcp_Pool> tcp_pool;
        std::map<std::string, std::string> headers;

        boost::future<Http_Response> request(const std::string& method,
                const std::string& path, boost::asio::const_buffer& body);

        // Formats an HTTP message including the given method, path, and any set headers.
        std::string create_request_head(const std::string& method, const std::string& path);

        Http_Response make_response(std::string& data);

}; // class Http

} // namespace net

#endif
