#include "logger.h"

#include <memory>

#include "boost_config.h"
#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

namespace logging = boost::log;
namespace sinks = logging::sinks;
namespace keywords = logging::keywords;

using boost::log::sinks::text_ostream_backend;

using net::Logger;

using std::shared_ptr;
using std::make_shared;

Logger::Logger()
{
    boost::shared_ptr< logging::core > core = logging::core::get();

    // Create backends
    text_backend = boost::make_shared< text_ostream_backend >();
    text_backend->add_stream(
                    boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter()));

    // Rotates every 12 hours or after a 5 MiB file size
    file_backend = boost::make_shared< sinks::text_file_backend >(
        keywords::file_name = "file_%5N.log", // filename
        keywords::rotation_size = 5 * 1024 * 1024, // 5 MiB rotates size
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) // 12 hour rotations
    );

    // Wrap inside of frontend
    text_frontend = boost::make_shared<async_frontend_text_sink>(text_backend);
    file_frontend = boost::make_shared<async_frontend_file_sink>(file_backend);

    // Attach to core
    core->add_sink(text_frontend);
    core->add_sink(file_frontend);
}

Logger::~Logger()
{
    boost::shared_ptr< logging::core > core = logging::core::get();

    // Remove the sink from the core, so that no records are passed to it.
    core->remove_sink(text_frontend);
    core->remove_sink(file_frontend);

    // Break feeding loop.
    text_frontend->stop();
    file_frontend->stop();

    // Flush any buffered records.
    text_frontend->flush();
    file_frontend->flush();

    text_frontend.reset();
    file_frontend.reset();
}
