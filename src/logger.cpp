#include "logger.h"

#include "spdlog/spdlog.h"

#include "boost_config.h"

using net::Logger;

Logger::Logger()
{
    log = spdlog::stdout_logger_mt("console", true /*use color*/);

    set_level(spdlog::level::trace);
}

Logger& Logger::get()
{
    static Logger l;
    return l;
}
