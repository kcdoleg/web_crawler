#pragma once

#include "crawler_exception/crawler_exception.h"
#include "logger/logger.h"


/* Main class for managing the whole process of crawling.
 * Allows to set arbitrary number of workers that executing certain task.
 */
class Crawler
{
public:
    Crawler(Logger& logger);

    void initThreadsNum(int fetchersNum, int parsersNum, int saversNum);
    void initVisitingParams(const std::string& startUrl, int maxDepth, const std::string& substrFilter = "");
    void setOutputDirectory(const std::string& outputDir);        

    void run();

private:
    void checkParamsIsOk() const;

private:
    Logger& m_logger;

    int m_fetchersNum;
    int m_parsersNum;
    int m_saversNum;

    std::string m_startUrl;
    int m_maxDepth;
    std::string m_substrFilter;

    std::string m_outputDir;
};

