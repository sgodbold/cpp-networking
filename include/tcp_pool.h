#ifndef CPP_NETWORKING_TCP_POOL_H
#define CPP_NETWORKING_TCP_POOL_H

#include "tcp.h"

#include <chrono>
#include <memory>
#include <queue>
#include <string>

#include "boost_definitions.h"
#include <boost/asio/deadline_timer.hpp>

/* Tcp_Pool
 *
 * Overview:
 * This class manages and caches a collection of TCP connections to a single host.
 *
 * Behavior:
 * Every connection in the pool has a timer which will disconnect the connection once
 * the time is up. Call the get method to acquire a connection (it may already be in
 * the pool or newly created). All connections taken out of the pool are protected by
 * a guard. Guards safely return a the protected connection to the pool upon destruction.
 * A user of this class only needs to call get() and start using the connection! Everything
 * else is taken care of.
 *
 * Design:
 * Implemented the named creation design patterns. Instantiation happens through a static
 * create method so that users can only every have a shared pointer to the object. This
 * simplifies the programming TCP Guards which operate independently after creation.
 * Guards can be used even after the pool is destroyed, the only difference is that the
 * protected connection has no pool to return to and will get destroyed along with the
 * guard.
 *
 */

namespace net
{

class Tcp_Pool
{
    public:
        class Tcp_Guard; // forward declared

        static std::shared_ptr<Tcp_Pool> create_new_pool(const std::string& host,
                                                         const std::string& service,
                                                         int timeout_milli);

        ~Tcp_Pool();

        Tcp_Guard get();

    private:
        Tcp_Pool(const std::string& host,
                 const std::string& service,
                 int milliseconds_timeout);

        void put_connection(std::unique_ptr<net::Tcp> tcp_client);

        void remove_connection();

        // Queue of cached connections. The top connection is the oldest. The bottom
        // connection is the youngest.
        std::queue< std::pair< net::Tcp, boost::asio::deadline_timer > > connections;
        std::mutex connection_lock;

        const std::string host;
        const std::string service;
        const boost::posix_time::ptime timout;
};

class Tcp_Pool::Tcp_Guard
{
    public:
        Tcp_Guard(std::unique_ptr<net::Tcp>&& tcp_client,
                  std::weak_ptr<net::Tcp_Pool> tcp_client_owner);

        // Destroying the guard will automatically place the tcp
        // connection back in the pool and reset the timeout period.
        ~Tcp_Guard();

    private:
        std::unique_ptr<net::Tcp> tcp_client;
        std::weak_ptr<Tcp_Pool> tcp_client_owner;
};

} // net

#endif
