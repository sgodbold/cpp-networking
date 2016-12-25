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

        //XXX should these async entry points be pass by value to ensure nothing is destructed before work is complete?

        boost::future<Http_Response> get(const std::string& url);

        boost::future<Http_Response> post(const std::string& url, boost::asio::const_buffer& body);

        boost::future<Http_Response> head(const std::string& url);

        boost::future<Http_Response> put(const std::string& url, boost::asio::const_buffer& body);

        boost::future<Http_Response> delet(const std::string& url);

        boost::future<Http_Response> trace(const std::string& url);

        boost::future<Http_Response> options(const std::string& url);

        boost::future<Http_Response> connect(const std::string& url, boost::asio::const_buffer& body);

        boost::future<Http_Response> patch(const std::string& url, boost::asio::const_buffer& body);

        // Setters used to construct every request
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

        boost::future<Http_Response> request(const std::string& method, const std::string& url,
                                             boost::asio::const_buffer& body);

        // Formats an HTTP message including the given method, url, and any set headers.
        boost::asio::const_buffer make_request(const std::string& method, const std::string& url);

        Http_Response make_response(std::string& data);

}; // class Http

} // namespace net

#endif
