#include "logs/logger.h"
#include "logs/log_sinks.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace sinks = logging::sinks;
namespace keywords = logging::keywords;

using net::Logger;
using net::Log_Sinks;

// Initialize statics
// Log_Sinks& Logger::sinks = Log_Sinks::create();
bool Logger::initialized = false;

Logger::Logger()
{
    if (!initialized)
    {
        initialized = true;

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

        logging::add_common_attributes();
    }
}
