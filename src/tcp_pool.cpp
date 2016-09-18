#include "tcp_pool.h"
#include "logger.h"

#include <string>

#include "boost_config.h"
#include <boost/bind.hpp>

using boost::asio::deadline_timer;
using boost::posix_time::time_duration;
using boost::system::error_code;

using net::Tcp;
using net::Tcp_Pool;

using std::bind;
using std::lock_guard;
using std::make_shared;
using std::make_unique;
using std::move;
using std::mutex;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::weak_ptr;

shared_ptr<Tcp_Pool> Tcp_Pool::create(const string& host,
                                      const string& service,
                                      time_duration duration)
{
    return shared_ptr<Tcp_Pool>( new Tcp_Pool(host, service, duration) );
}

Tcp_Pool::~Tcp_Pool()
{
}

// Either create a new connection to return or grab a connection from the pool,
// stop its timer, and return.
//
// XXX should there be a 'max connections' where once reached get() will block
// until a connection is put back in the pool?
Tcp_Pool::Tcp_Guard Tcp_Pool::get()
{
    // lock_guard<mutex> lck(connections_lock);

    unique_ptr<Tcp> client;

    // If there are no connections in the pool create a new one.
    if (connections.empty())
    {
        Logger::get()->debug("Tcp_Pool: creating new connection for the pool");
        client = make_unique<Tcp>(host, service);
    }
    // Else grab a connection from the pool.
    else
    {
        Logger::get()->debug("Tcp_Pool: using cached connection");

        // Grab the top connection.
        Timed_Tcp_Connection_t ttc = std::move(connections.front());
        connections.pop();

        // Stop its deadline timer.
        ttc.second->cancel();

        client = move(ttc.first);
    }

    weak_ptr<Tcp_Pool> wp(shared_from_this());
    Tcp_Guard tcp_guard(move(client), wp);
    return tcp_guard;
}

Tcp_Pool::Tcp_Pool(const string& host_, const string& service_, time_duration duration_)
  : host(host_),
    service(service_),
    timeout(duration_),
    io_service_manager(Io_Service_Manager::Behavior_t::Perpetual)
{
}

void Tcp_Pool::put_connection(std::unique_ptr<Tcp> tcp_client)
{
    lock_guard<mutex> lck(connections_lock);
    Logger::get()->trace("Tcp_Pool::put_connection()");

    // Create and start the deadline timer.
    auto deadline = make_shared<deadline_timer>(io_service_manager.get());
    deadline->expires_from_now(timeout);
    deadline->async_wait(boost::bind(&Tcp_Pool::handle_remove_connection, this, boost::asio::placeholders::error));
    Timed_Tcp_Connection_t ttc { move(tcp_client), std::move(deadline) };

    // Add timed connection back to queue.
    connections.push(std::move(ttc));
}

void Tcp_Pool::handle_remove_connection(const error_code& ec)
{
    // If there's an error immediately return because deadline.cancel() was called somewhere.
    if (ec)
    {
        Logger::get()->debug("Tcp_Pool::handle_remove_connection {}", ec.message());
        return;
    }
    else
    {
        remove_connection();
    }
}

void Tcp_Pool::remove_connection()
{
    lock_guard<mutex> lck(connections_lock);
    Logger::get()->trace("Tcp_Pool::remove_connection()");

    // Check timer of top connection to see if it's expired
    auto ttc_ref = &connections.front();
    if (ttc_ref->second->expires_at() <= deadline_timer::traits_type::now())
    {
        // Remove connection from queue. This closes and destroys the connection.
        connections.pop();
    }
    else
    {
        Logger::get()->warn("Tcp_Pool: remove_connection() triggered but no connection is expired");
    }
}

Tcp_Pool::Tcp_Guard::Tcp_Guard(std::unique_ptr<Tcp>&& tcp_client_,
                               std::weak_ptr<Tcp_Pool>tcp_client_owner_)
  : tcp_client(move(tcp_client_)),
    tcp_client_owner(tcp_client_owner_)
{}

// If the connection pool still exists and the connection is still open,
// it will be put back. Otherwise it will be disconnected during destruction.
Tcp_Pool::Tcp_Guard::~Tcp_Guard()
{
    auto sp = tcp_client_owner.lock();
    if (sp and tcp_client->status() == Tcp::Status_t::Open)
    {
        Logger::get()->debug("Tcp_Pool: putting connection back in the pool");
        sp->put_connection(move(tcp_client));
        return;
    }

    // Not putting connection back, so just close it.
    tcp_client.reset();

    if(!sp)
    {
        Logger::get()->debug("Tcp_Guard: owner pool is destroyed; closing connection");
    }
    else
    {
        Logger::get()->debug("Tcp_Guard: connection is closed; leaving out of pool");
    }
}
