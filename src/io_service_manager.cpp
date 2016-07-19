#include "io_service_manager.h"

#include <algorithm>
#include <memory>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

using boost::asio::io_service;
using boost::future;
using boost::thread;

using net::Io_Service_Manager;

using std::lock_guard;
using std::logic_error;
using std::make_shared;
using std::mutex;
using std::unique_lock;

Io_Service_Manager::Io_Service_Manager() : Io_Service_Manager(Behavior_t::Default) {}

Io_Service_Manager::Io_Service_Manager(Behavior_t b)
    : behavior(b), status(Status_t::Stopped), io_thread_worker()
{
    if (is_perpetual())
    {
        start();
    }
}

Io_Service_Manager::~Io_Service_Manager()
{
    if (is_running())
    {
        stop();
    }

    if (io_thread_worker and io_thread_worker->joinable())
    {
        io_thread_worker->join();
    }
}

// Start service by adding the first worker.
void Io_Service_Manager::start()
{
    if (is_running())
    {
        throw logic_error("Io_Service_Manager is already running");
    }

    if (is_perpetual())
    {
        io_work = make_shared<io_service::work>(io_service);
    }

    io_thread_worker = make_shared<thread>(std::bind(&Io_Service_Manager::run_worker, this));
    
    status = Status_t::Running;
}

// Stop all worker threads and reset the service to leave in a state
// where it can be started again.
void Io_Service_Manager::stop()
{
    if (!is_running())
    {
        throw logic_error("Io_Service_Manager is already stopped");
    }

    if (is_perpetual())
    {
        io_work.reset();
    }

    // Stop service and all worker threads.
    io_service.stop();

    if (io_thread_worker and io_thread_worker->joinable())
    {
        io_thread_worker->join();
        io_thread_worker.reset();
    }

    // Prepare service to startup again.
    io_service.reset();

    status = Status_t::Stopped;
}

void Io_Service_Manager::run_worker()
{
    // Blocks until all work is done.
    // If this Io_Service_Manager is perpetual it will run until either:
    //     1. io_work is destroyed, and all work is completed
    //     2. io_service is stopped
    Io_Service_Manager::io_service.run();

    // This will be the only worker. Notify that all work is complete.
    lock_guard<mutex> lck(io_threads_lock);
    io_threads_empty_cv.notify_all();
}

void Io_Service_Manager::block_until_stopped()
{
    unique_lock<mutex> lck(io_threads_lock);

    while (status != Status_t::Stopped)
    {
        io_threads_empty_cv.wait(lck);
    }
}
