#ifndef CPP_NETWORKING_TCP_POOL_H
#define CPP_NETWORKING_TCP_POOL_H

#include "io_service_manager.h"
#include "tcp.h"

#include <chrono>
#include <memory>
#include <queue>
#include <string>

#include "boost_config.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

/* Tcp_Pool and Tcp_Guard
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

class Tcp_Pool : public std::enable_shared_from_this<Tcp_Pool>
{
    public:
        class Tcp_Guard; // forward declared

        using Timed_Tcp_Connection_t = std::pair<std::unique_ptr<net::Tcp>,
                                                 std::shared_ptr<boost::asio::deadline_timer>>;

        static std::shared_ptr<Tcp_Pool> create(
                const std::string& host,
                const std::string& service,
                boost::posix_time::time_duration duration);

        ~Tcp_Pool();

        Tcp_Guard get();

    private:
        Tcp_Pool(const std::string& host,
                 const std::string& service,
                 boost::posix_time::time_duration duration);

        void put_connection(std::unique_ptr<net::Tcp> tcp_client);

        void remove_connection();

        // Queue of cached connections. The top connection is the oldest. The bottom
        // connection is the youngest.
        std::queue<Timed_Tcp_Connection_t> connections;
        std::mutex connections_lock;

        const std::string host;
        const std::string service;
        const boost::posix_time::time_duration timeout;

        // Needed to execute deadline timers and disconnect connections.
        Io_Service_Manager io_service_manager;
};

class Tcp_Pool::Tcp_Guard
{
    public:
        Tcp_Guard(std::unique_ptr<net::Tcp>&& tcp_client,
                  std::weak_ptr<net::Tcp_Pool> tcp_client_owner);

        // Destroying the guard will automatically place the tcp
        // connection back in the pool and reset the timeout period.
        ~Tcp_Guard();

        Tcp_Guard(Tcp_Guard&& other) = default;
        Tcp_Guard& operator=(Tcp_Guard&& other) = default;

        // Disallow copy construction and assignment.
        Tcp_Guard(const Tcp_Guard&) = delete;
        Tcp_Guard& operator=(const Tcp_Guard&) = delete;

        // XXX is this an unsafe way to allow access for function calls on the client?
        // would an interface be better? A benefit to this method is single point of
        // maintenance on function signature changes.
        Tcp& operator*()
            { return *tcp_client; }
        Tcp* operator->()
            { return &(*tcp_client); }

    private:
        std::unique_ptr<net::Tcp> tcp_client;
        std::weak_ptr<Tcp_Pool> tcp_client_owner;
};

} // net

#endif
