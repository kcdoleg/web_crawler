#pragma once

#include "logger/logger.h"

#include <mutex>
#include <unordered_set>


class ThreadContext
{
public:
    ThreadContext(Logger& logger)
        : m_isWorkDone(false)
        , m_logger(logger)
    {

    }

    void setWorkDone()
    {
        m_isWorkDone = true;
    }

    bool isWorkDone() const
    {
        return m_isWorkDone;
    }

    std::mutex& getMutex()
    {
        return m_mutex;
    }

    Logger& getLogger()
    {
        return m_logger;
    }

private:
    mutable std::mutex m_mutex;
    bool m_isWorkDone;
    Logger& m_logger;
};


class FetcherThreadContext: public ThreadContext
{
public:
    FetcherThreadContext(Logger& logger,
        const std::shared_ptr<std::unordered_set<std::string>>& visitedUrls)
        : ThreadContext(logger)
        , m_visitedUrls(visitedUrls)
    {

    }

    bool isVisited(const std::string& md5sum) const
    {
        return m_visitedUrls->count(md5sum);
    }

    void addVisited(const std::string& md5sum)
    {
        m_visitedUrls->insert(md5sum);
    }

private:
    std::shared_ptr<std::unordered_set<std::string>> m_visitedUrls;
};


class ParserThreadContext: public ThreadContext
{
public:
    ParserThreadContext(Logger& logger, const std::string& filter)
        : ThreadContext(logger)
        , m_filter(filter)
    {

    }

    const std::string& getFilter() const
    {
        return m_filter;
    }

private:
    std::string m_filter;
};


class SaverThreadContext: public ThreadContext
{
public:
    SaverThreadContext(Logger& logger, const std::string& outputDir)
        : ThreadContext(logger)
        , m_outputDir(outputDir)
        , m_storedTotal(0)
    {

    }

    const std::string& getOutputDirectory() const
    {
        return m_outputDir;
    }

    void addStored()
    {
        ++m_storedTotal;
    }

    int getStoredTotal() const
    {
        return m_storedTotal;
    }

private:
    std::string m_outputDir;
    int m_storedTotal;
};


