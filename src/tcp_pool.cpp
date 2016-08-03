#include "tcp_pool.h"

#include <string>

using net::Tcp;
using net::Tcp_Pool;

using std::make_shared;
using std::move;
using std::shared_ptr;
using std::string;

/* Tcp_Pool Public */

shared_ptr<Tcp_Pool> Tcp_Pool::create_new_pool(const string& host,
                                               const string& service,
                                               int timeout_milli)
{
    return shared_ptr<Tcp_Pool>( new Tcp_Pool(host, service, timeout_milli) );
}

Tcp_Pool::~Tcp_Pool()
{
}

Tcp_Pool::Tcp_Guard Tcp_Pool::get()
{
    // lock pool
    //
    // if theres cached connections
    //      pop from queue & cancel timer
    //      create guard
    //      return guard
    // else
    //      create tcp connection
    //      create guard
    //      return guard
}

void Tcp_Pool::put_connection(std::unique_ptr<Tcp> tcp_client)
{
    // lock pool
    //
    // create and start timer
    // push to queue
}


/* Tcp_Pool Private */

Tcp_Pool::Tcp_Pool(const string& host, const string& service, int milliseconds_timeout)
{
}

void Tcp_Pool::remove_connection()
{
    // lock pool
    //
    // check timer of top connection.
    // if expired
    //      pop, disconnect, destroy
    // else
    //      log an error but leave alone?
}

Tcp_Pool::Tcp_Guard::Tcp_Guard(std::unique_ptr<Tcp>&& tcp_client_,
                               std::weak_ptr<Tcp_Pool>tcp_client_owner_)
    : tcp_client(move(tcp_client_)), tcp_client_owner(tcp_client_owner_)
{}

// If the connection pool still exists the connection is put back.
// Otherwise it will be disconnected during destruction.
Tcp_Pool::Tcp_Guard::~Tcp_Guard()
{
    auto sp = tcp_client_owner.lock();
    if(sp)
    {
        sp->put_connection(move(tcp_client));
    }
}
