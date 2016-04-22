#include "http.h"

#include <boost/asio.hpp>
#include <ostream>
#include <string>
#include <vector>

using std::string;

using boost::asio::const_buffer; using boost::asio::streambuf;
using boost::system::error_code; using boost::system::system_error;
using boost::asio::read_until;

using async_net::Http;

// Use same buffer for sending requests without a body.
static const_buffer no_body;
static Http::Http_Response no_response;

Http::Http(const string& host_)
    : connection(host_, "http"), host(host_), http_version(http_version_)
{
    // Request that the server closes the socket after sending a response.
    add_header("Connection", "close");
}

void Http::request(Method_t method, const std::string& path, const_buffer& body, Http_Handler_t h)
{
    string method_str = method_to_str(method);
    const_buffer req = make_request(method_str, path);

    std::vector<const_buffer> req_sequence;
    req_sequence.push_back(req);
    req_sequence.push_back(body);

    connection.send(req_sequence, [this, h](boost::system::error_code& error)
    {
        // There might not be anything to receive XXX?
        // XXX would this be a bug where a send errors, but there's still
        // data in the receive buffer that isn't retreived until the next
        // call?
        if(error) {
            h(error, no_response);
            return;
        }

        connection.receive([this, h](error_code& error, const_buffer data)
        {
            Http_Response res = (error) ? no_response : make_response(data);
            h(error, res);
        });
    });
}

void Http::get(const string& path, Http_Handler_t h)
{
    request(Method_t::GET, path, no_body, h);
    // XXX cache response?
}

void Http::head(const string& path, Http_Handler_t h)
{
    request(Method_t::HEAD, path, no_body, h);
    // XXX cache response?
}

void Http::post(const string& path, const_buffer& body, Http_Handler_t h)
{
    request(Method_t::POST, path, body, h);
    // XXX cache response?
}

void Http::put(const string& path, const_buffer& body, Http_Handler_t h)
{
    request(Method_t::PUT, path, body, h);
}

void Http::delet(const string& path, Http_Handler_t h)
{
    request(Method_t::DELETE, path, no_body, h);
}

void Http::trace(const string& path, Http_Handler_t h)
{
    request(Method_t::TRACE, path, no_body, h);
}

void Http::options(const string& path, Http_Handler_t h)
{
    request(Method_t::OPTIONS, path, no_body, h);
}

void Http::connect(const string& path, const_buffer& body, Http_Handler_t h)
{
    request(Method_t::CONNECT, path, body, h);
}

void Http::patch(const string& path, const_buffer& body, Http_Handler_t h)
{
    request(Method_t::PATCH, path, body, h);
    // XXX cache response?
}

/* Private */

// Formats an HTTP message including the given method, path, and any set headers.
const_buffer Http::make_request(const std::string& method, const std::string& path)
{
    streambuf req;
    std::ostream request_stream(&req);

    // Method
    request_stream << method << " " << path << " " << http_version << "\r\n";

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

    // Status
    streambuf b;
    read_until(data, b, "\r\n");

    // XXX handling errors??

    // Headers

    // Body

    return response;
}

string Http::method_to_str(Method_t m)
{
    switch(m) {
        case Method_t::GET:
            return "GET";
        case Method_t::POST:
            return "POST";
        case Method_t::HEAD:
            return "HEAD";
        case Method_t::PUT:
            return "PUT";
        case Method_t::DELETE:
            return "DELETE";
        case Method_t::TRACE:
            return "TRACE";
        case Method_t::OPTIONS:
            return "OPTIONS";
        case Method_t::CONNECT:
            return "CONNECT";
        case Method_t::PATCH:
            return "PATCH";
    };
}
