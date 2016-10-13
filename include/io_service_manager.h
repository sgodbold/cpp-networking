#ifndef CPP_NETWORKING_IO_SERVICE_H
#define CPP_NETWORKING_IO_SERVICE_H

/* Io_Service_Manager
 *
 * Overview:
 * This class manages a boost io_service object to simplify the operations of
 * running one.
 *
 * Behaviors:
 * Default: io_service object automatically stops once queued work is complete
 * Perpetual: io_service object must be manually stopped
 *
 * Design:
 * There are 4 states (constructing, destructing, running, stopped) with respective
 * functions to move between the states. State changes must be atomic given how many
 * threads are involved in io work.
 *
 */

// XXX Should this interface be modified to protect access from any worker thread?
//     It could be an accidental call in a lambda it's executing such as closing a TCP connection
//     which by default stops the Io_Service_Manager. Every function could be protected with
//     a thread id check. If this_thread_id in Io_Service_Manager::set<thread::id>; throw.

#include <condition_variable>
#include <memory>
#include <mutex>

#include "boost_config.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace net
{

class Io_Service_Manager
{
    public:
        enum class Behavior_t
        {
            Default,
            Perpetual,
        };

        // Create a default io_service. Does not start automatically.
        Io_Service_Manager();

        // Create a specific io_service. If set to perpetual then one worker
        // thread will automatically start running jobs.
        // Atomically changes the state depending on behavior
        Io_Service_Manager(Behavior_t);

        // Blocks until completely stopped.
        // Atomically changes the state to stopped then to destructing
        ~Io_Service_Manager();

        // Start running the io_service. This runs one worker thread.
        // Throws logic_error if already started.
        // Atomically changes the state
        void start();

        // Stops the io_service and all worker threads.
        // Throws logic_error if already stopped.
        // Atomically changes the state
        void stop();

        boost::asio::io_service& get()
            { return io_service; }

        boost::asio::io_service::strand create_strand()
        { return boost::asio::io_service::strand(io_service); }

        bool is_running()
            { return state == State_t::Running; }

        bool is_perpetual()
            { return behavior == Behavior_t::Perpetual; }

        void block_until_stopped();
        void block_until_running();

    private:
        enum class State_t
        {
            Running,
            Stopped,
        };

        void run_worker();

        // Changes to the start state. Only call this with the state change lock acquired.
        void to_start_state();

        // Changes to the stop state. Only call this with the state change lock acquired.
        void to_stop_state();

        Behavior_t behavior;
        State_t state;

        boost::asio::io_service io_service;
        std::shared_ptr<boost::asio::io_service::work> io_work;
        std::shared_ptr<boost::thread> io_thread_worker;

        // Locks
        std::mutex state_change_lock;

        // Conditions
        std::condition_variable service_stopped_cv;
        std::condition_variable service_started_cv;

}; // Io_Service_Manager

} // net


#endif
