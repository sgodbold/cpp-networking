#include "http.h"

#include <future>
#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/future.hpp>

using boost::asio::const_buffer; using boost::asio::streambuf;
using boost::asio::read_until;
using boost::future;
using boost::system::error_code; using boost::system::system_error;

using std::string;
using std::vector;

using net::Http;
using net::Http_Response;

// Given a buffer with HTTP response data this returns a single line
// stipped of the ending \r\n.
static string get_response_line(const_buffer& data);

// Use same buffer for empty contents in requests and responses.
static const_buffer no_body;

Http::Http(const string& host_)
    : connection(host_, "http"), host(host_), http_version(http_version_)
{
    // Request that the server closes the socket after sending a response.
    add_header("Connection", "close");
}

// XXX now that sends / receives are asynchronous all requests / responses must be atomically
// paired up. 1 thread can't process multiple responses from the same socket. Maybe have a 
// queue for 1 socket or connect a new socket for every request?
future<Http_Response> Http::request(const std::string& method, const std::string& path,
                                          const_buffer& body)
{
    boost::promise<Http_Response> prom;
    future<Http_Response> fut = prom.get_future();

    const_buffer req = make_request(method, path);

    std::vector<const_buffer> req_sequence;
    req_sequence.push_back(req);
    req_sequence.push_back(body);

    // Send request then receive response then notify the returned future.
    error_code send_ec;
    connection.send(req, send_ec).then([&](future<size_t> f) -> void {
        size_t len = f.get();

        if(send_ec) {
            // receive error response / clear socket buffer?
        }

        error_code recv_ec;
        connection.receive(recv_ec).then([&](future<const_buffer> f) -> void {
            const_buffer data = f.get();
            
            if(recv_ec) {
                // XXX
            }
            
            Http_Response res = make_response(data);

            // Signal caller that the request is done.
            prom.set_value(res);

            return;
        });
    });

    return fut;
}

future<Http_Response> Http::get(const string& path)
{
    return request("GET", path, no_body);
    // XXX cache response?
}

future<Http_Response> Http::head(const string& path)
{
    return request("HEAD", path, no_body);
    // XXX cache response?
}

future<Http_Response> Http::post(const string& path, const_buffer& body)
{
    return request("POST", path, body);
    // XXX cache response?
}

future<Http_Response> Http::put(const string& path, const_buffer& body)
{
    return request("PUT", path, body);
}

future<Http_Response> Http::delet(const string& path)
{
    return request("DELETE", path, no_body);
}

future<Http_Response> Http::trace(const string& path)
{
    return request("TRACE", path, no_body);
}

future<Http_Response> Http::options(const string& path)
{
    return request("OPTIONS", path, no_body);
}

future<Http_Response> Http::connect(const string& path, const_buffer& body)
{
    return request("CONNECT", path, body);
}

future<Http_Response> Http::patch(const string& path, const_buffer& body)
{
    return request("PATCH", path, body);
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

Http_Response Http::make_response(boost::asio::const_buffer& data)
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

        // XXX what if the line isn't a valid header?
        // check for proper form of re.match(".+:.+", line)

        // Strip any surrounding whitespace
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
