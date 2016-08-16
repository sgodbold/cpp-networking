#define BOOST_TEST_DYN_LINK

#include "io_service_manager.h"

#include "boost_definitions.h"
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/future.hpp>

using boost::future;
using boost::promise;

using net::Io_Service_Manager;

struct A_Running_Perpetual_Service
{
    A_Running_Perpetual_Service() : service(Io_Service_Manager::Behavior_t::Perpetual)
    {}

    Io_Service_Manager service;
};

struct A_Stopped_Perpetual_Service : public A_Running_Perpetual_Service
{
    A_Stopped_Perpetual_Service() : A_Running_Perpetual_Service()
    {
        service.stop();
    }
};

BOOST_AUTO_TEST_SUITE( io_service_manager_suite )

    BOOST_AUTO_TEST_SUITE( default_behavior )
        // TODO
    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE( perpetual_behavior )

        BOOST_FIXTURE_TEST_SUITE( running_state, A_Running_Perpetual_Service )

            BOOST_AUTO_TEST_CASE( check_service_is_perpetual )
            {
                BOOST_TEST( service.is_perpetual() );
            }

            BOOST_AUTO_TEST_CASE( check_service_is_running )
            {
                BOOST_TEST( service.is_running() );
            }

            BOOST_AUTO_TEST_CASE( check_start_throws_when_already_running )
            {
                BOOST_CHECK_THROW( service.start(), std::logic_error );
            }

            BOOST_AUTO_TEST_CASE( check_get_doesnt_throw )
            {
                BOOST_CHECK_NO_THROW( service.get() );
            }

            BOOST_AUTO_TEST_CASE( check_stopping_works )
            {
                BOOST_CHECK_NO_THROW( service.stop() );
                BOOST_TEST( !service.is_running() );
            }

            struct With_Queued_Work : public A_Running_Perpetual_Service
            {
                With_Queued_Work() : A_Running_Perpetual_Service()
                {
                    fut = prom.get_future();
                    service.get().dispatch([&]()
                        {
                            prom.set_value(true);
                        }
                    );
                }

                promise<bool> prom;
                future<bool> fut;
            };

            BOOST_FIXTURE_TEST_CASE( check_that_work_is_done_correctly, With_Queued_Work)
            {
                bool val = fut.get();
                BOOST_TEST( val == true );
            }

            BOOST_FIXTURE_TEST_CASE( check_io_service_is_still_running, With_Queued_Work)
            {
                BOOST_TEST( service.is_running() );
            }

        BOOST_AUTO_TEST_SUITE_END() // running_state

        BOOST_FIXTURE_TEST_SUITE( stopped_state, A_Stopped_Perpetual_Service )

            BOOST_AUTO_TEST_CASE( check_service_is_perpetual )
            {
                BOOST_TEST( service.is_perpetual() );
            }

            BOOST_AUTO_TEST_CASE( check_service_is_not_running )
            {
                BOOST_TEST( !service.is_running() );
            }

            BOOST_AUTO_TEST_CASE( check_stopping_throws )
            {
                BOOST_CHECK_THROW( service.stop(), std::logic_error );
            }

            BOOST_AUTO_TEST_CASE( check_starting_works )
            {
                BOOST_CHECK_NO_THROW( service.start() );
                BOOST_CHECK( service.is_running() );
            }

            struct With_Queued_Work : public A_Stopped_Perpetual_Service
            {
                With_Queued_Work() : A_Stopped_Perpetual_Service()
                {
                    fut = prom.get_future();
                    service.get().dispatch([&]()
                        {
                            prom.set_value(true);
                        }
                    );
                }

                promise<bool> prom;
                future<bool> fut;
            };

            BOOST_FIXTURE_TEST_CASE( check_work_is_not_done_until_started, With_Queued_Work )
            {
                BOOST_TEST( !fut.is_ready() );

                service.start();

                BOOST_TEST( fut.get() == true );
            }

        BOOST_AUTO_TEST_SUITE_END() // stopped_state

    BOOST_AUTO_TEST_SUITE_END() // perpetual_behavior

BOOST_AUTO_TEST_SUITE_END() // io_service_manager_suite
