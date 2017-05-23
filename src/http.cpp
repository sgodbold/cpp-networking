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

using boost::asio::buffer;
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

static const char* get_c = "GET";
static const char* post_c = "POST";
static const char* head_c = "HEAD";
static const char* put_c = "PUT";
static const char* delete_c = "DELETE";
static const char* trace_c = "TRACE";
static const char* options_c = "OPTIONS";
static const char* connect_c = "CONNECT";
static const char* patch_c = "PATCH";

string create_request_head(const string& method, const string& path);

// Given a buffer with HTTP response data this returns a single line
// stipped of the ending \r\n.
static string get_response_line(string& data)
{
    // TODO
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

future<Http_Response> Http::request(const string& method, const string& path, const_buffer& body)
{
    boost::promise<Http_Response> prom;
    future<Http_Response> fut = prom.get_future();

    auto tcp_client = tcp_pool.get();
    error_code send_ec;
    // string request = create_request(method, path);

    // auto send_fut = tcp_client->send(request, send_ec);
    // send_fut.then([this, tcp_client, send_ec]

    return fut;
}

/*
future<Http_Response> Http::request(const string& method, const string& path, const string& body)
{
    auto prom = make_shared<promise<shared_ptr<Http_Request>>>();

    auto tcp_client = tcp_pool.get();
    error_code ec;

    // XXX Request shouldn't be on stack
    string request = create_request(method, path);
    request += body;
    size_t req_size = request.size();

    tcp_client->send(request, ec).then(
        [this, tcp_client, ec, req_size] (future<size_t> send_fut)
    {
        // Ensure data sent properly
        size_t sent = send_fut.get();

        if(ec)
        {
            // XXX
        }
        else
        {
            ec.clear();
        }

        if(sent != request.size())
        {
            // XXX
        }

        // Receive status line
        return tcp_client->receive("\n", ec);
    }).then([this, tcp_client, ec] (future<shared_ptr<string> recv_fut)
    {
        // Receive status line containing <VERSION> <CODE> <PHRASE>
        shared_ptr<string> status_line = recv_fut.get();
    });

    return prom->get_future();
}
*/

/*
future<Http_Response> receive(shared_ptr<Tcp_Guard> tcp_client)
{
}
*/

future<Http_Response> Http::get(const string& path)
{
    string req_head = create_request_head(get_c, path);
    return request("GET", path, no_body);
    // XXX cache response?
}

future<Http_Response> Http::head(const string& path)
{
    string req_head = create_request_head(head_c, path);
    return request("HEAD", path, no_body);
    // XXX cache response?
}

future<Http_Response> Http::post(const string& path, const_buffer& body)
{
    string req_head = create_request_head(post_c, path);
    return request("POST", path, body);
    // XXX cache response?
}

future<Http_Response> Http::post(const string& path, const string& body)
{
    string req_head = create_request_head(post_c, path);
    
    const_buffer send_buf(buffer(body));
}

future<Http_Response> Http::put(const string& path, const_buffer& body)
{
    string req_head = create_request_head(put_c, path);
    return request("PUT", path, body);
}

future<Http_Response> Http::delet(const string& path)
{
    string req_head = create_request_head(delete_c, path);
    return request("DELETE", path, no_body);
}

future<Http_Response> Http::trace(const string& path)
{
    string req_head = create_request_head(trace_c, path);
    return request("TRACE", path, no_body);
}

future<Http_Response> Http::options(const string& path)
{
    string req_head = create_request_head(options_c, path);
    return request("OPTIONS", path, no_body);
}

future<Http_Response> Http::connect(const string& path, const_buffer& body)
{
    string req_head = create_request_head(connect_c, path);

    return request("CONNECT", path, body);
}

future<Http_Response> Http::patch(const string& path, const_buffer& body)
{
    string req_head = create_request_head(patch_c, path);

    return request("PATCH", path, body);
    // XXX cache response?
}

string Http::create_request_head(const string& method, const string& path)
{
    // Method
    string req = method + " " + path + " " + http_version_c + "\r\n";

    // Headers
    // XXX does this copy the headers or reference it?
    for (auto h : headers)
    {
        // req << h.first << ": " << h.second << "\r\n";
    }

    // Spacer
    // req << "\r\n";

    return req;
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
