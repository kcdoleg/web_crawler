#pragma once

#include <iostream>
#include <sstream>
#include <mutex>


enum SeverityLevel
{
    SL_TRACE = 0,
    SL_DEBUG = 1,
    SL_WARNING = 2,
    SL_INFO = 3,
    SL_ERROR = 4
};


/* Simple safe-thread logging class.
 * Supports artibtrary stream output and several severity levels.
 */
class Logger
{
public:
    Logger(std::ostream& output, SeverityLevel level);

    void log(SeverityLevel level, const std::string& msg);

private:
    std::ostream& m_output;
    SeverityLevel m_severityLevel;
    std::mutex m_mutex;
};


#define SIMPLE_LOG(logger, level, msg) \
{ \
    std::ostringstream os; \
    os << msg; \
    logger.log(level, os.str()); \
}
