/*
 *          Copyright Andrey Semashev 2007 - 2013.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "logs/logger.h"

#include "boost_config.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

/*
//[ example_tutorial_file_simple
void init()
{
    logging::add_file_log("sample.log");

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
}
//]

// We need this due to this bug: https://svn.boost.org/trac/boost/ticket/4416
//[ example_tutorial_file_advanced_no_callouts
void init()
{
    logging::add_file_log
    (
        keywords::file_name = "sample_%N.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = "[%TimeStamp%]: %Message%"
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
}
//]
*/

//[ example_tutorial_file_advanced
void init()
{
    logging::add_file_log
    (
        keywords::file_name = "sample_%N.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = "[%TimeStamp%]: %Message%"
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
}
//]

int main(int, char*[])
{
    /*
    init();
    logging::add_common_attributes();

    src::severity_logger< severity_level > lg;
    */

    using namespace logging::trivial;

    net::Logger l;

    BOOST_LOG_SEV(l.log, trace) << "A trace severity message";
    BOOST_LOG_SEV(l.log, debug) << "A debug severity message";
    BOOST_LOG_SEV(l.log, info) << "An informational severity message";
    BOOST_LOG_SEV(l.log, warning) << "A warning severity message";
    BOOST_LOG_SEV(l.log, error) << "An error severity message";
    BOOST_LOG_SEV(l.log, fatal) << "A fatal severity message";

    return 0;
}
