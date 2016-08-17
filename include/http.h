#ifndef CPP_NETWORKING_HTTP_H
#define CPP_NETWORKING_HTTP_H

#include "tcp.h"

#include <map>
#include <string>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

// Settings
const char* http_version_ = "HTTP/1.1";

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

        //XXX should these async entry points be pass by value to ensure nothing is destructed before work is complete?

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

        // Setters used to construct every request
        void add_header(const std::string& name, const std::string& value)
            { headers[name] = value; }
        void remove_header(const std::string& name)
            { headers.erase(name); }
        void reset_headers()
            { headers.clear(); }

    private:
        Tcp tcp_client;
        const std::string& host;
        std::string http_version;
        std::map<std::string, std::string> headers;

        boost::asio::const_buffer make_request(const std::string& method, const std::string& path);

        Http_Response make_response(std::string& data);

}; // class Http
} // namespace net

#endif
