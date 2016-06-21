#ifndef CPP_NETWORKING_IO_SERVICE_H
#define CPP_NETWORKING_IO_SERVICE_H

#include <memory>
#include <vector>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace net
{

class Io_Service
{
    public:
        enum class Behavior_t
        {
            Default,
            Perpetual,
        };

        Io_Service();

        Io_Service(Behavior_t);

        ~Io_Service();

        void start();

        void stop();

        void add_worker_thread();

        boost::asio::io_service& get()
            { return io_service; }

        bool is_running()
            { return status == Status_t::Running; }

        bool is_perpetual()
            { return behavior == Behavior_t::Perpetual; }

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
        std::vector<boost::thread> io_threads;
}; // Io_Service

} // net


#endif
