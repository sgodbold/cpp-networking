#include "http.h"

#include <boost/asio.hpp>
#include <ostream>
#include <string>
#include <vector>

using std::string;

using boost::asio::const_buffer; using boost::asio::streambuf;
using boost::system::error_code; using boost::system::system_error;

using async_net::Http;

// Use same buffer for sending requests without a body.
static const_buffer no_body;

Http::Http(const string& host_) : connection(host_, "http"), host(host_), protocol(protocol_)
{
    // Request that the server closes the socket after sending a response.
    add_header("Connection", "close");
}

void Http::request(const string& method, const std::string& path,
                                  const_buffer& body, Http_Handler h)
{
    const_buffer req = make_request(method, path);
    std::vector<const_buffer> req_sequence;
    req_sequence.push_back(req);
    req_sequence.push_back(body);

    connection.send(req_sequence, [&](boost::system::error_code& error)
        {
            // XXX receive data, parse response, call handler
        }
    );
}

void Http::get(const string& path, Http_Handler h)
{
    request("GET", path, no_body, h);
    // XXX cache response?
}

void Http::head(const string& path, Http_Handler h)
{
    request("HEAD", path, no_body, h);
    // XXX cache response?
}

void Http::post(const string& path, const_buffer& body, Http_Handler h)
{
    request("POST", path, body, h);
    // XXX cache response?
}

void Http::put(const string& path, const_buffer& body, Http_Handler h)
{
    request("PUT", path, body, h);
}

void Http::delet(const string& path, Http_Handler h)
{
    request("DELETE", path, no_body, h);
}

void Http::trace(const string& path, Http_Handler h)
{
    request("TRACE", path, no_body, h);
}

void Http::options(const string& path, Http_Handler h)
{
    request("OPTIONS", path, no_body, h);
}

void Http::connect(const string& path, const_buffer& body, Http_Handler h)
{
    request("CONNECT", path, body, h);
}

void Http::patch(const string& path, const_buffer& body, Http_Handler h)
{
    request("PATCH", path, body, h);
    // XXX cache response?
}

/* Private */

// Formats an HTTP message including the given method, path, and any set headers.
const_buffer Http::make_request(const std::string& method, const std::string& path)
{
    streambuf req;
    std::ostream request_stream(&req);

    // Method
    request_stream << method << " " << path << " " << protocol << "\r\n";

    // Headers
    for (auto& h : headers) {
        request_stream << h.first << ": " << h.second << "\r\n";
    }

    // Spacer
    request_stream << "\r\n";

    return req.data();
}

Http::Http_Response Http::make_response(boost::asio::const_buffer data)
{
    Http_Response response;
    return response;
}
