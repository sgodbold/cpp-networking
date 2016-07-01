#include "io_service_manager.h"

#include <catch.hpp>
#include <memory>
#include <string>

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

using boost::future;
using boost::promise;

using net::Io_Service_Manager;

SCENARIO("Using a default io service manager", "[io_service_manager][default][nowork]")
{
    GIVEN("io service manager with no queued work")
    {
        Io_Service_Manager service;

        /* Check initial state */

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

        /*
        AND_WHEN("a worker is added")
        {
            service.add_worker();

            THEN("io_service remains stopped")
            {
                service.block_until_work_complete();
                CHECK(!service.is_running());
            }
        }
        */

        WHEN("manager is started")
        {
            service.start();
            service.block_until_work_complete();

            THEN("io_service remains stopped")
            {
                CHECK(!service.is_running());
            }

            THEN("starting the service again doesn't throw")
            {
                CHECK_NOTHROW(service.start());
            }
        }
    }

    /*
    GIVEN("io_service with queued work")
    {
        Io_Service_Manager service;

        promise<bool> prom;
        future<bool> fut = prom.get_future();

        service.get().dispatch([&]()
            {
                prom.set_value(true); 
            }
        );

        WHEN("io_serice is started")
        {
            service.start();

            // If this blocks, then the service was never started.
            // XXX how do I assert this?
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
    }
    */
}
