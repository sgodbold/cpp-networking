#include "io_service_manager.h"

#include "logger.h"

#include <algorithm>
#include <memory>

#include "boost_config.h"
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
    : behavior(b),
      state(State_t::Stopped),
      io_thread_worker()
{
    // Top level state change function. Everything must be atomic.
    lock_guard<mutex> lck(state_change_lock);

    if (is_perpetual())
    {
        to_start_state();
    }
}

Io_Service_Manager::~Io_Service_Manager()
{
    // Top level state change function. Everything must be atomic.
    lock_guard<mutex> lck(state_change_lock);

    if (is_running())
    {
        to_stop_state();
    }

    // Even if workers are no longer running, the threads still need to be joined
    // to properly clean them up.
    if (io_thread_worker and io_thread_worker->joinable())
    {
        io_thread_worker->join();
    }
}

void Io_Service_Manager::start()
{
    // Top level state change function. Everything must be atomic.
    lock_guard<mutex> lck(state_change_lock);
    to_start_state();
    service_started_cv.notify_all();
}

void Io_Service_Manager::stop()
{
    // Top level state change function. Everything must be atomic.
    lock_guard<mutex> lck(state_change_lock);
    to_stop_state();
    service_stopped_cv.notify_all();
}

// Start service by adding the first worker.
void Io_Service_Manager::to_start_state()
{
    if (is_running())
    {
        logic_error e("Io_Service_Manager is already running");
        Logger::get()->warn(e.what());
        throw e;
    }

    if (is_perpetual())
    {
        io_work = make_shared<io_service::work>(io_service);
    }

    io_thread_worker = make_shared<thread>(std::bind(&Io_Service_Manager::run_worker, this));
    
    state = State_t::Running;
}

// Stop all worker threads and reset the service to leave in a state
// where it can be started again.
void Io_Service_Manager::to_stop_state()
{
    if (!is_running())
    {
        logic_error e("Io_Service_Manager is already stopped");
        Logger::get()->debug(e.what());
        throw e;
    }

    // Destroy the io_work so that a perpetual io_service knows to stop.
    if (is_perpetual())
    {
        io_work.reset();
    }

    // Stop service and all worker threads.
    io_service.stop();

    // Stop and join worker threads.
    if (io_thread_worker and io_thread_worker->joinable())
    {
        io_thread_worker->join();
        io_thread_worker.reset();
    }

    // Prepare service to startup again.
    io_service.reset();

    state = State_t::Stopped;
}

void Io_Service_Manager::run_worker()
{
    // Blocks until all work is done.
    // If this Io_Service_Manager is perpetual it will run until either:
    //     1. io_work is destroyed, and all work is completed
    //     2. io_service is stopped
    Io_Service_Manager::io_service.run();

    // XXX not clean
    std::string state_str;
    switch (state)
    {
        case State_t::Destructing:
            state_str = "destructing";
            break;
        case State_t::Running:
            state_str = "running";
            break;
        case State_t::Stopped:
            state_str = "stopped";
            break;
    }

    Logger::get()->trace("Io_Service_Manager: worker exiting at state: {}", state_str);
}

void Io_Service_Manager::block_until_stopped()
{
    unique_lock<mutex> lck(state_change_lock);

    while (state != State_t::Stopped)
    {
        service_stopped_cv.wait(lck);
    }
}

void Io_Service_Manager::block_until_running()
{
    unique_lock<mutex> lck(state_change_lock);

    while (state != State_t::Stopped)
    {
        service_started_cv.wait(lck);
    }
}
