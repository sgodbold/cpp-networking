#include "http.h"

#include <boost/asio.hpp>
#include <ostream>
#include <string>
#include <vector>

using std::string;

net::http::http(const string& host_) : connection(host_, "http"), host(host_), protocol(protocol_)
{
    // Request that the server closes the socket after sending a response.
    add_header("Connection", "close");
}

net::http::http_response net::http::request(const string& method, const std::string& path,
                                            boost::asio::const_buffer& body)
{
    boost::asio::const_buffer req = make_request(method, path);
    connection.send({req, body});
}

net::http::http_response net::http::request(const string& method, const std::string& path)
{
    boost::asio::const_buffer req = make_request(method, path);
    connection.send(req);
}

void net::http::get(const string& path)
{
    request("GET", path);
}

void net::http::post(const string& path, boost::asio::const_buffer& body)
{
    request("POST", path, body);
}

/* Private */

// Formats an HTTP message including the given method, path, and any set headers.
boost::asio::const_buffer net::http::make_request(const std::string& method,
                                                  const std::string& path)
{
    boost::asio::streambuf req;
    std::ostream request_stream(&req);

    // Method
    request_stream << method << path << protocol << "\r\n";

    // Headers
    for (auto& h : headers) {
        request_stream << h.first << ": " << h.second << "\r\n";
    }

    // Spacer
    request_stream << "\r\n";

    return req.data();
}
