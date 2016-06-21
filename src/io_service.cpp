#include "io_service.h"

#include <algorithm>
#include <memory>

#include "boost_definitions.h"
#include <boost/asio.hpp>

using boost::asio::io_service;

using net::Io_Service;

using std::make_shared;

Io_Service::Io_Service() : Io_Service(Behavior_t::Default) {}

Io_Service::Io_Service(Behavior_t b)
    : behavior(b), status(Status_t::Stopped)
{
    start();
}

Io_Service::~Io_Service()
{
    if (is_running())
    {
        stop();
    }
}

void Io_Service::start()
{
    // XXX what if behavior is default and the worker threads are done??

    if (is_running())
    {
        throw std::logic_error("Io_Service is already running");
    }

    if (is_perpetual())
    {
        io_work = make_shared<io_service::work>(io_service);
    }

    add_worker_thread();

    status = Status_t::Running;
}

void Io_Service::stop()
{
    if (!is_running())
    {
        throw std::logic_error("Io_Service is already stopped");
    }

    if (is_perpetual())
    {
        io_work.reset();
    }

    // Stop all worker threads.
    io_service.stop();
    for_each(io_threads.begin(), io_threads.end(), [](boost::thread& t)
        {
            t.join();
        }
    );

    // Prepare service to startup again.
    io_service.reset();

    status = Status_t::Stopped;
}

void Io_Service::add_worker_thread()
{
    io_threads.push_back(boost::thread(&Io_Service::run_worker, this));
}

void Io_Service::run_worker()
{
    // Blocks until all work is done.
    // If this Io_Service is perpetual it will run until either:
    //     1. io_work is destroyed, and all work is completed
    //     2. io_service is stopped
    Io_Service::io_service.run();

    // XXX remove thread from io_threads container. Once container is empty set status to stopped?
}
