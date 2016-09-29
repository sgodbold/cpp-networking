#ifndef CPP_NETWORKING_LOGS_LOGGER_H
#define CPP_NETWORKING_LOGS_LOGGER_H

/* Logger
 *
 * Overview:
 * A global logging singleton.
 *
 * Usage:
 * Call Logger::get()->severity(string) to log a message at different severity levels.
 *  ->trace();
 *  ->debug();
 *  ->info();
 *  ->warn();
 *  ->critical();
 *
 * To filter messages of a certain severity out of the logs
 * just call Logger::set_level(severity level).
 *
 */

#include "spdlog/spdlog.h"

#include <memory>

#include "boost_config.h"

namespace net
{

class Logger
{
    public:
        ~Logger()
        {}

        static Logger& get();

        static void set_level(spdlog::level::level_enum l)
            { spdlog::set_level(l); }

        spdlog::logger& operator*()
            { return *log; }
        spdlog::logger* operator->()
            { return &(*log); }

    private:
        // Disallow public construction.
        Logger();

        std::shared_ptr<spdlog::logger> log;

}; // Logger

} // net

#endif
