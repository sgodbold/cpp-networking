#ifndef CPP_NETWORKING_LOGS_LOG_SINKS_H
#define CPP_NETWORKING_LOGS_LOG_SINKS_H

#include <memory>

namespace net
{

class Log_Sinks
{
    public:
        static Log_Sinks& create();

        ~Log_Sinks();

    private:
        // Disallow public construction
        Log_Sinks();

}; // Log_Sinks

}

#endif
