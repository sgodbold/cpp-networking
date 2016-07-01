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
using std::make_shared;
using std::mutex;
using std::unique_lock;

Io_Service_Manager::Io_Service_Manager() : Io_Service_Manager(Behavior_t::Default) {}

Io_Service_Manager::Io_Service_Manager(Behavior_t b)
    : behavior(b), status(Status_t::Stopped)
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
}

void Io_Service_Manager::start()
{
    if (is_running())
    {
        throw std::logic_error("Io_Service_Manager is already running");
    }

    if (is_perpetual())
    {
        io_work = make_shared<io_service::work>(io_service);
    }

    add_worker();
}

void Io_Service_Manager::stop()
{
    if (!is_running())
    {
        throw std::logic_error("Io Service is already stopped");
    }

    if (is_perpetual())
    {
        io_work.reset();
    }

    io_service.stop();

    // Join all worker threads and clear thread container
    for_each(io_threads.begin(), io_threads.end(), boost::mem_fn(&thread::join));
    io_threads.clear();

    // Prepare service to startup again.
    io_service.reset();

    status = Status_t::Stopped;
}

void Io_Service_Manager::add_worker()
{
    lock_guard<mutex> lck(io_threads_lock);

    thread t(&Io_Service_Manager::run_worker, this);
    io_threads.push_back(std::move(t));
    status = Status_t::Running;
}

void Io_Service_Manager::run_worker()
{
    // Blocks until all work is done.
    // If this Io_Service is perpetual it will run until either:
    //     1. io_work is destroyed, and all work is completed
    //     2. io_service is stopped
    Io_Service_Manager::io_service.run();

    remove_worker(boost::this_thread::get_id());
}

void Io_Service_Manager::remove_worker(thread::id id)
{
    lock_guard<mutex> lck(io_threads_lock);

    // Find and remove thread object with given id.
    auto erase_it = find_if(io_threads.begin(), io_threads.end(),
        [id](const boost::thread& t)
        {
            return t.get_id() == id;
        }
    );

    if (erase_it == io_threads.end())
    {
        throw std::logic_error("Can't remove worker thread. Id not found");
    }
    else
    {
        // XXX this thread deleting it's own thread object causes SIGABRT
        io_threads.erase(erase_it);
    }

    if(io_threads.empty())
    {
        status = Status_t::Stopped;
        io_threads_empty_cv.notify_all();
    }
}

void Io_Service_Manager::block_until_work_complete()
{
    unique_lock<mutex> lck(io_threads_lock);

    while (!io_threads.empty())
    {
        io_threads_empty_cv.wait(lck);
    }
}
