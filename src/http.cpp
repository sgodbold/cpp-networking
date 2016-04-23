#include "http.h"

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

using std::string;
using std::size_t;

using boost::asio::const_buffer; using boost::asio::streambuf;
using boost::system::error_code; using boost::system::system_error;
using boost::asio::read_until;

using async_net::Http;

// Given a buffer with HTTP response data this returns a single line
// stipped of the ending \r\n.
static string get_response_line(const_buffer& data);

// Use same buffer for sending requests without a body.
static const_buffer no_body;
static Http::Http_Response no_response;

Http::Http(const string& host_)
    : connection(host_, "http"), host(host_), http_version(http_version_)
{
    // Request that the server closes the socket after sending a response.
    add_header("Connection", "close");
}

void Http::request(const std::string& method, const std::string& path,
                   const_buffer& body, Http_Handler_t h)
{
    const_buffer req = make_request(method, path);

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
    request("GET", path, no_body, h);
    // XXX cache response?
}

void Http::head(const string& path, Http_Handler_t h)
{
    request("HEAD", path, no_body, h);
    // XXX cache response?
}

void Http::post(const string& path, const_buffer& body, Http_Handler_t h)
{
    request("POST", path, body, h);
    // XXX cache response?
}

void Http::put(const string& path, const_buffer& body, Http_Handler_t h)
{
    request("PUT", path, body, h);
}

void Http::delet(const string& path, Http_Handler_t h)
{
    request("DELETE", path, no_body, h);
}

void Http::trace(const string& path, Http_Handler_t h)
{
    request("TRACE", path, no_body, h);
}

void Http::options(const string& path, Http_Handler_t h)
{
    request("OPTIONS", path, no_body, h);
}

void Http::connect(const string& path, const_buffer& body, Http_Handler_t h)
{
    request("CONNECT", path, body, h);
}

void Http::patch(const string& path, const_buffer& body, Http_Handler_t h)
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
    Http_Response res;

    // Status
    res.status = get_response_line(data);

    // Headers
    string line;
    while (true) {
        line = get_response_line(data);
        if (line.empty()) {
            break;
        }
        std::vector<string> tokens;
        boost::split(tokens, line, boost::is_any_of(":"));

        // XXX what is the line isn't a valid header?

        boost::trim(tokens[0]);
        boost::trim(tokens[1]);

        res.headers[tokens[0]] = tokens[1];
    }

    // Body
    res.body = data;

    return res;
}

/* Helpers */

// Given a buffer with HTTP response data this returns a single line
// stipped of the ending \r\n.
string get_response_line(const_buffer& data)
{
    streambuf buf;
    size_t n = read_until(data, buf, "\r\n");
    buf.commit(n);
    std::istream is(&buf);

    string result;
    is >> result;

    // strip the ending \r\n
    result.pop_back();
    result.pop_back();

    return result;
}
