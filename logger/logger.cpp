#include "logger.h"

#include <ctime>
#include <cstdio>


Logger::Logger(std::ostream& output, SeverityLevel level)
    : m_output(output)
    , m_severityLevel(level)
{

}


static const char* levelStr[] = {
    "TRACE",
    "DEBUG",
    "WARNING",
    "INFO",
    "ERROR"
};


void Logger::log(SeverityLevel level, const std::string& msg)
{
    if (level >= m_severityLevel) {
        std::time_t rawtime;
        std::tm* timeinfo;
        char buffer[80];
        std::time(&rawtime);
        timeinfo = std::localtime(&rawtime);
        std::strftime(buffer, 80, "%Y-%m-%d-%H-%M-%S", timeinfo);
        std::unique_lock<std::mutex> lck(m_mutex);
        m_output << buffer << " : " << levelStr[level] << " : " << msg << std::endl;
    }
}

