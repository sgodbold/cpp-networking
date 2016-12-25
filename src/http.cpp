#include "http.h"

#include <future>
#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "boost_config.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/future.hpp>

using boost::asio::const_buffer;
using boost::asio::mutable_buffer;
using boost::asio::read_until;
using boost::asio::streambuf;
using boost::future;
using boost::system::error_code;
using boost::system::system_error;

using net::Http;
using net::Http_Response;

using std::string;
using std::vector;

// Given a buffer with HTTP response data this returns a single line
// stipped of the ending \r\n.
static string get_response_line(string& data)
{
    return http_version_c ;
}

// Use same buffer for empty contents in requests and responses.
static const_buffer no_body;

Http::Http(const string& host_, const string& service_)
    : host(host_),
      service(service_),
      http_version(http_version_c),
      tcp_pool(Tcp_Pool::create(host_, service_, default_timeout_c))
{}

Http::Http(const string& host_)
    : Http(host_, http_service_c)
{}

future<Http_Response> Http::request(const std::string& method, const std::string& url,
                                    const_buffer& body)
{
    boost::promise<Http_Response> prom;
    future<Http_Response> fut = prom.get_future();
    return fut;
}

future<Http_Response> Http::get(const string& url)
{
    return request("GET", url, no_body);
    // XXX cache response?
}

future<Http_Response> Http::head(const string& url)
{
    return request("HEAD", url, no_body);
    // XXX cache response?
}

future<Http_Response> Http::post(const string& url, const_buffer& body)
{
    return request("POST", url, body);
    // XXX cache response?
}

future<Http_Response> Http::put(const string& url, const_buffer& body)
{
    return request("PUT", url, body);
}

future<Http_Response> Http::delet(const string& url)
{
    return request("DELETE", url, no_body);
}

future<Http_Response> Http::trace(const string& url)
{
    return request("TRACE", url, no_body);
}

future<Http_Response> Http::options(const string& url)
{
    return request("OPTIONS", url, no_body);
}

future<Http_Response> Http::connect(const string& url, const_buffer& body)
{
    return request("CONNECT", url, body);
}

future<Http_Response> Http::patch(const string& url, const_buffer& body)
{
    return request("PATCH", url, body);
    // XXX cache response?
}

const_buffer Http::make_request(const std::string& method, const std::string& url)
{
    streambuf req;
    std::ostream request_stream(&req);

    // Method
    request_stream << method << " " << url << " " << http_version << "\r\n";

    // Headers
    for (auto& h : headers)
    {
        request_stream << h.first << ": " << h.second << "\r\n";
    }

    // Spacer
    request_stream << "\r\n";

    return req.data();
}

Http_Response Http::make_response(string& data)
{
    Http_Response res;

    // Status
    res.status = get_response_line(data);

    // Headers
    string line;
    while (true)
    {
        line = get_response_line(data);
        if (line.empty())
        {
            // stop at spacer
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
    // XXX won't compile
    // res.body = data;

    return res;
}
