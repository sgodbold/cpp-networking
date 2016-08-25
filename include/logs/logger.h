#ifndef CPP_NETWORKING_LOGS_LOGGER_H
#define CPP_NETWORKING_LOGS_LOGGER_H

#include "logs/log_sinks.h"

#include <memory>

#include "boost_config.h"
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>

namespace net
{

class Logger
{
    public:
        Logger();

        ~Logger()
        {}

        boost::log::sources::severity_logger< boost::log::trivial::severity_level > log;

    private:
        // Log_Sinks& sinks;

        static bool initialized;

}; // Logger

} // net

#endif
