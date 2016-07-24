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
    Io_Service_Manager service;

    THEN("it is not perpetual")
    {
        CHECK(!service.is_perpetual());
    }

    THEN("getting the internal service won't throw")
    {
        CHECK_NOTHROW(service.get());
    }

    THEN("it is in the stopped state")
    {
        CHECK(!service.is_running());
    }

    THEN("stopping it throws")
    {
        CHECK_THROWS(service.stop());
    }

    GIVEN("no queued work")
    {
        WHEN("the service is started")
        {
            service.start();

            THEN("the service remains in the stopped state")
            {
                CHECK(!service.is_running());
            }
        }
    }

    GIVEN("queued work")
    {
        promise<bool> prom;
        future<bool> fut = prom.get_future();

        service.get().dispatch([&]()
            {
                prom.set_value(true); 
            }
        );

        WHEN("the service is started")
        {
            service.start();

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
    }
}

SCENARIO("Using a perpetual io service manager", "[io_service_manager][perpetual]")
{
    Io_Service_Manager service(Io_Service_Manager::Behavior_t::Perpetual);

    THEN("it is in the running state")
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

    GIVEN("no queued work")
    {
        WHEN("the service is stopped")
        {
            service.stop();

            THEN("it is in the stopped state")
            {
                CHECK(!service.is_running());
            }

            THEN("stopping it throws")
            {
                CHECK_THROWS(service.stop());
            }

            THEN("it is perpetual")
            {
                CHECK(service.is_perpetual());
            }

            AND_WHEN("the service is started again")
            {
                service.start();

                THEN("it is in the running state")
                {
                    CHECK(service.is_running());
                }

                THEN("it is perpetual")
                {
                    CHECK(service.is_perpetual());
                }
            }
        }
    }

    GIVEN("queued work")
    {
        promise<bool> prom;
        future<bool> fut = prom.get_future();

        service.get().dispatch([&]()
            {
                prom.set_value(true); 
            }
        );

        // If this blocks, then the service was never started. XXX how do I assert this?
        bool val = fut.get();

        THEN("the work done is correct")
        {
            CHECK(val == true);
        }

        THEN("io_service is still running")
        {
            CHECK(service.is_running());
        }

        WHEN("the service is stopped")
        {
            service.stop();

            THEN("it is in the stopped state")
            {
                CHECK(!service.is_running());
            }

            AND_WHEN("more work is added and the service is started again")
            {
                promise<bool> prom;
                future<bool> fut = prom.get_future();

                service.get().dispatch([&]()
                    {
                        prom.set_value(true); 
                    }
                );

                service.start();

                // If this blocks, then the service was never started. XXX how do I assert this?
                bool val = fut.get();

                THEN("it is in the running state")
                {
                    CHECK(service.is_running());
                }

                THEN("it is perpetual")
                {
                    CHECK(service.is_perpetual());
                }

                THEN("the work done is correct")
                {
                    CHECK(val == true);
                }
            }
        }

    }
}
