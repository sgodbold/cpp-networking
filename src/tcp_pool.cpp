#include "tcp_pool.h"

#include <string>

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

/* Tcp_Pool Public */

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
// XXX should there be a 'max connections' where once reached get() will block
// until a connection is put back in the pool?
Tcp_Pool::Tcp_Guard Tcp_Pool::get()
{
    // lock_guard<mutex> lck(connections_lock);

    unique_ptr<Tcp> client;

    // If there are no connections in the pool create a new one.
    if (connections.empty())
    {
        std::cout << "creating new connection for the pool" << std::endl;
        client = make_unique<Tcp>(host, service);
    }
    // Else grab a connection from the pool.
    else
    {
        std::cout << "using existing connection" << std::endl;
        // Grab the top connection.
        Timed_Tcp_Connection_t ttc = std::move(connections.front());
        connections.pop();

        // Stop its deadline timer.
        ttc.second->cancel();

        client = std::move(ttc.first);
    }

    weak_ptr<Tcp_Pool> wp(shared_from_this());
    Tcp_Guard tcp_guard(move(client), wp);
    return tcp_guard;
}


/* Tcp_Pool Private */

Tcp_Pool::Tcp_Pool(const string& host_, const string& service_, time_duration duration_)
    : host(host_), service(service_), timeout(duration_),
    io_service_manager(Io_Service_Manager::Behavior_t::Perpetual)
{
}

void Tcp_Pool::put_connection(std::unique_ptr<Tcp> tcp_client)
{
    lock_guard<mutex> lck(connections_lock);

    // Create and start the deadline timer.
    auto deadline = make_shared<deadline_timer>(io_service_manager.get());
    deadline->expires_from_now(timeout);
    deadline->async_wait(bind(&Tcp_Pool::remove_connection, this));
    Timed_Tcp_Connection_t ttc { move(tcp_client), deadline };

    // Add timed connection back to queue.
    connections.push(std::move(ttc));
}

void Tcp_Pool::remove_connection()
{
    lock_guard<mutex> lck(connections_lock);

    // Check timer of top connection to see if it's expired
    auto ttc_ref = &connections.front();
    if (ttc_ref->second->expires_at() <= deadline_timer::traits_type::now())
    {
        // Remove connection from queue. This closes and destroys the connection.
        connections.pop();
    }
    // XXX else
    //      log an error but leave alone?
}

Tcp_Pool::Tcp_Guard::Tcp_Guard(std::unique_ptr<Tcp>&& tcp_client_,
                               std::weak_ptr<Tcp_Pool>tcp_client_owner_)
    : tcp_client(move(tcp_client_)), tcp_client_owner(tcp_client_owner_)
{}

// If the connection pool still exists and the connection is still open,
// it will be put back. Otherwise it will be disconnected during destruction.
Tcp_Pool::Tcp_Guard::~Tcp_Guard()
{
    std::cout << "destructing guard" << std::endl;
    auto sp = tcp_client_owner.lock();
    if (sp and tcp_client->status() == Tcp::Status_t::Open)
    {
        std::cout << "putting connection back" << std::endl;
        sp->put_connection(move(tcp_client));
    }
}
