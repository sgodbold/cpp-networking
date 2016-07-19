#ifndef CPP_NETWORKING_IO_SERVICE_H
#define CPP_NETWORKING_IO_SERVICE_H

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

#include "boost_definitions.h"
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
        // Blocks until completely started
        Io_Service_Manager(Behavior_t);

        // Blocks until completely stopped.
        ~Io_Service_Manager();

        // Start running the io_service. This runs one worker thread.
        // Throws logic_error if already started.
        // Blocks until completely started
        void start();

        // Stops the io_service and all worker threads.
        // Throws logic_error if already stopped.
        // Blocks until completely stopped.
        void stop();

        // Adds a new worker thread to run jobs.
        // TODO: void add_worker();

        boost::asio::io_service& get()
            { return io_service; }

        bool is_running()
            { return status == Status_t::Running; }

        bool is_perpetual()
            { return behavior == Behavior_t::Perpetual; }

        void block_until_stopped();

    private:
        enum class Status_t
        {
            Running,
            Stopped,
        };

        void run_worker();

        Behavior_t behavior;

        Status_t status;

        boost::asio::io_service io_service;
        std::shared_ptr<boost::asio::io_service::work> io_work;

        // Tracks a set of all worker threads.
        // Workers remove themselves from the set once done.
        std::shared_ptr<boost::thread> io_thread_worker;
        std::mutex io_threads_lock;
        std::condition_variable io_threads_empty_cv;

}; // Io_Service_Manager

} // net


#endif
