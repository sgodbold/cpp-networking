#include "io_service_manager.h"

#include <algorithm>
#include <memory>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/mem_fn.hpp>
#include <boost/thread/future.hpp>

using boost::asio::io_service;
using boost::future;
using boost::mem_fn;
using boost::thread;

using net::Io_Service_Manager;

using std::lock_guard;
using std::logic_error;
using std::make_shared;
using std::mutex;
using std::unique_lock;

Io_Service_Manager::Io_Service_Manager() : Io_Service_Manager(Behavior_t::Default) {}

Io_Service_Manager::Io_Service_Manager(Behavior_t b)
    : behavior(b), status(Status_t::Stopped), stopped_threads(0)
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

    if (io_thread_workers.size() > 0)
    {
        // XXX: hacky!
        io_thread_workers.join_all();
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

    add_worker();
}

// Stop all worker threads and reset the service to leave in a state
// where it can be started again.
void Io_Service_Manager::stop()
{
    lock_guard<mutex> lck(io_threads_lock);

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
    io_thread_workers.join_all();
    stopped_threads = 0; // XXX hacky!
    io_threads_empty_cv.notify_all();

    // Prepare service to startup again.
    io_service.reset();

    status = Status_t::Stopped;
}

void Io_Service_Manager::add_worker()
{
    lock_guard<mutex> lck(io_threads_lock);
    io_thread_workers.create_thread(std::bind(&Io_Service_Manager::run_worker, this));
}

void Io_Service_Manager::run_worker()
{
    status = Status_t::Running;

    // Blocks until all work is done.
    // If this Io_Service_Manager is perpetual it will run until either:
    //     1. io_work is destroyed, and all work is completed
    //     2. io_service is stopped
    Io_Service_Manager::io_service.run();

    lock_guard<mutex> lck(io_threads_lock);
    if (io_thread_workers.size() == ++stopped_threads) // XXX hacky!
    {
        status = Status_t::Stopped;
        io_threads_empty_cv.notify_all();
    }
}

void Io_Service_Manager::block_until_work_complete()
{
    unique_lock<mutex> lck(io_threads_lock);

    while (io_thread_workers.size() != stopped_threads)
    {
        io_threads_empty_cv.wait(lck);
    }
}
