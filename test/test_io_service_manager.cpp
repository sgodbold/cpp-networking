#include "io_service_manager.h"

#include <catch.hpp>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

using boost::future;
using boost::promise;

using net::Io_Service_Manager;

SCENARIO("Using a default io service manager", "[io_service_manager][default]")
{
    GIVEN("no queued work")
    {
        Io_Service_Manager service;

        THEN("it is not running")
        {
            CHECK(!service.is_running());
        }

        THEN("it is not perpetual")
        {
            CHECK(!service.is_perpetual());
        }

        THEN("stopping it throws")
        {
            CHECK_THROWS(service.stop());
        }

        THEN("getting the internal service won't throw")
        {
            CHECK_NOTHROW(service.get());
        }

        WHEN("the manager is started")
        {
            service.start();
            service.block_until_stopped();

            THEN("io_service remains stopped")
            {
                CHECK(!service.is_running());
            }

            /*
            THEN("starting the service again doesn't throw")
            {
                CHECK_NOTHROW(service.start());
                service.block_until_work_is_complete();
            }
            */
        }
    }

    GIVEN("queued work")
    {
        Io_Service_Manager service;

        promise<bool> prom;
        future<bool> fut = prom.get_future();

        service.get().dispatch([&]()
            {
                prom.set_value(true); 
            }
        );

        WHEN("io_service is started")
        {
            service.start();
            service.block_until_stopped();

            // If this blocks, then the service was never started. XXX how do I assert this?
            bool val = fut.get();

            THEN("the work done is correct")
            {
                CHECK(val == true);
            }

            THEN("io_service is now stopped")
            {
                CHECK(!service.is_running());
            }
        }

        /* TODO: add the restarting test case
        WHEN("that work is done, more work is added, and the service is started again")
        {
        }
        */
    }
}

SCENARIO("Using a perpetual io service manager", "[io_service_manager][perpetual]")
{
    GIVEN("no queued work")
    {
        Io_Service_Manager service(Io_Service_Manager::Behavior_t::Perpetual);

        std::cout << service.is_running() << std::endl;

        THEN("it is running")
        {
            CHECK(service.is_running());
        }

        THEN("it is perpetual")
        {
            CHECK(service.is_perpetual());
        }

        THEN("starting it throws")
        {
            CHECK_THROWS(service.start());
        }

        THEN("getting the internal service doesn't throw")
        {
            CHECK_NOTHROW(service.get());
        }

        /*
        WHEN("the service is stopped")
        {
            service.stop(); // XXX blocks

            THEN("it is no longer running")
            {
                CHECK(!service.is_running());
            }
        }
        */
    }
}
