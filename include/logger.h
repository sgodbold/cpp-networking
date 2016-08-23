#ifndef CPP_NETWORKING_LOGGER_H
#define CPP_NETWORKING_LOGGER_H

#include "boost_config.h"
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>

namespace net
{

class Logger
{
    public:
        enum class Severity_Level_t
        {
            Debug,
            Normal,
            Warning,
            Error,
            Critical,
        };

        Logger();
        ~Logger();

        // Loggers
        boost::log::sources::severity_logger_mt<Severity_Level_t> log;

        // Frontend Sinks
        using async_frontend_text_sink = boost::log::sinks::asynchronous_sink< boost::log::sinks::text_ostream_backend >;
        using async_frontend_file_sink = boost::log::sinks::asynchronous_sink< boost::log::sinks::text_file_backend >;

        boost::shared_ptr<async_frontend_text_sink> text_frontend;
        boost::shared_ptr<async_frontend_file_sink> file_frontend;

        // Backend Sinks
        boost::shared_ptr< boost::log::sinks::text_ostream_backend > text_backend;
        boost::shared_ptr< boost::log::sinks::text_file_backend > file_backend;

}; // Logger

} // net

#endif
