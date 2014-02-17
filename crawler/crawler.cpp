#include "crawler.h"
#include "url_utils.h"
#include "task_pool.h"
#include "execute_task.h"
#include "thread_context.h"
#include "curl_tools/tools.h"
#include "crawler_exception/crawler_exception.h"

#include <unordered_set>
#include <fstream>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


Crawler::Crawler(Logger& logger)
    : m_logger(logger)
    , m_fetchersNum(50)
    , m_parsersNum(4)
    , m_saversNum(4)
    , m_startUrl()
    , m_maxDepth(1)
    , m_substrFilter()
    , m_outputDir("./results")
{

}


void Crawler::initThreadsNum(int fetchersNum, int parsersNum, int saversNum)
{
    m_fetchersNum = fetchersNum;
    m_parsersNum = parsersNum;
    m_saversNum = saversNum;
}


void Crawler::initVisitingParams(const std::string& startUrl, int maxDepth,
    const std::string& substrFilter /*= ""*/)
{
    m_startUrl = startUrl;
    m_maxDepth = maxDepth;
    m_substrFilter = substrFilter;
}


void Crawler::setOutputDirectory(const std::string& outputDir)
{
    m_outputDir = outputDir;
}


void Crawler::checkParamsIsOk() const
{
    if (m_fetchersNum <= 0) {
        throw CrawlingParamsError("Unexpected number of fetchers");
    }
    if (m_parsersNum <= 0) {
        throw CrawlingParamsError("Unexpected number of parsers");
    }
    if (m_saversNum <= 0) {
        throw CrawlingParamsError("Unexpected number of savers");
    }
    if (m_startUrl.empty()) {
        throw CrawlingParamsError("Empty start url");
    }
    if (m_maxDepth <= 0) {
        throw CrawlingParamsError("Unexpected maximal depth");
    }
    if (m_outputDir.empty()) {
        throw CrawlingParamsError("Output directory name is empty");
    }
}


// Main struct for url and its text
struct FetchedUrl
{
    std::string url;
    std::shared_ptr<std::string> text;
    std::string md5;
};


// Implementation of Fetcher's work:
// Take one url, fetch it, calc md5 and put it to store pool and parse pool
void fetchUrlImpl(
    std::shared_ptr<FetcherThreadContext> threadContext,
    const std::string& url,
    std::shared_ptr<TaskPool<FetchedUrl>> parsingPool,
    std::shared_ptr<TaskPool<FetchedUrl>> storingPool)
{
    FetchedUrl fetchedUrl;
    fetchedUrl.url = url;
    fetchedUrl.text.reset(new std::string());
    try {
        fetchUrl(url, fetchedUrl.text.get());
    } catch (CurlException& e) {
        SIMPLE_LOG(threadContext->getLogger(), SL_WARNING, e.what());
        return;
    }
    fetchedUrl.md5 = calcMd5(*fetchedUrl.text);

    {
        std::unique_lock<std::mutex> visitedLock(threadContext->getMutex());
        if (threadContext->isVisited(fetchedUrl.md5)) {
            return;
        } else {
            threadContext->addVisited(fetchedUrl.md5);
        }
    }

    {
        std::unique_lock<std::mutex> parsingPoolLock(parsingPool->getMutex());
        parsingPool->addTask(fetchedUrl);
    }

    {
        std::unique_lock<std::mutex> storingPoolLock(storingPool->getMutex());
        storingPool->addTask(fetchedUrl);
    }
}


// Implementation of Parser's work:
// Take url with text, parse new urls from it and put them into result pool
void parseUrlImpl(
    std::shared_ptr<ParserThreadContext> threadContext,
    const FetchedUrl& fetchedUrl,
    std::shared_ptr<TaskPool<std::string>> refsPool)
{
    std::vector<std::string> parsed;
    parseUrls(*fetchedUrl.text, &parsed, threadContext->getFilter());

    {
        std::unique_lock<std::mutex> refsPoolLock(refsPool->getMutex());
        for (size_t i = 0; i < parsed.size(); ++i) {
            refsPool->addTask(parsed[i]);
        }
    }
}


// Implementation of Saver's work:
// Take url with text and store it to the harddrive.
void storeUrlImpl(
    std::shared_ptr<SaverThreadContext> threadContext,
    const FetchedUrl& fetchedUrl)
{
    std::string url = fetchedUrl.url;
    for (size_t i = 0; i < url.length(); ++i) {
        if ((url[i] < 'a' || url[i] > 'z') && url[i] != '.') {
            url[i] = '_';
        }
    }
    url += "_md5_" + fetchedUrl.md5;
    std::string outputDir = threadContext->getOutputDirectory();
    struct stat st = {0};
    if (stat(outputDir.c_str(), &st) == -1) {
        mkdir(outputDir.c_str(), 0777);
    }
    std::ofstream of(outputDir + "/" + url);
    of << *fetchedUrl.text;
    {
        std::unique_lock<std::mutex> contextLock(threadContext->getMutex());
        threadContext->addStored();
    }
}


template<class Task>
void waitForEmptyPool(TaskPool<Task>& taskPool)
{
    while (true) {
        std::unique_lock<std::mutex> lck(taskPool.getMutex());
        if (!taskPool.empty()) {
            taskPool.getEmptyCondition().wait(lck);
        } else {
            break;
        }
    }
}


void Crawler::run()
{
    checkParamsIsOk();
    
    SIMPLE_LOG(m_logger, SL_INFO, "Crawler is run. First url is \"" << m_startUrl << "\".");

    // Creating task pools
    std::shared_ptr<TaskPool<std::string>> urlToFetch(new TaskPool<std::string>());
    std::shared_ptr<TaskPool<FetchedUrl>> urlToParse(new TaskPool<FetchedUrl>());
    std::shared_ptr<TaskPool<std::string>> parsedUrls(new TaskPool<std::string>());
    std::shared_ptr<TaskPool<FetchedUrl>> urlToStore(new TaskPool<FetchedUrl>());

    // Mark startUrl as visited and add startUrl as first task to the pool
    std::shared_ptr<std::unordered_set<std::string>> visitedUrls(
        new std::unordered_set<std::string>());
    urlToFetch->addTask(m_startUrl);

    int totalUrlsFound = 0;
    int totalUrlsSaved = 0;
    // Begin iteration process by depth
    for (int depth = 0; depth < m_maxDepth; ++depth) {
 
        SIMPLE_LOG(m_logger, SL_INFO, "Visiting depth " << depth + 1 << ".");
 
        // Creating workers(fetchers, parsers and savers)
        std::vector<std::unique_ptr<std::thread>> fetchers;
        std::vector<std::unique_ptr<std::thread>> parsers;
        std::vector<std::unique_ptr<std::thread>> savers;
        // And their common contexts
        std::shared_ptr<FetcherThreadContext> fetcherContext(
            new FetcherThreadContext(m_logger, visitedUrls));
        std::shared_ptr<ParserThreadContext> parserContext(
            new ParserThreadContext(m_logger, m_substrFilter));
        std::shared_ptr<SaverThreadContext> saverContext(
            new SaverThreadContext(m_logger, m_outputDir));
 
        // Running workers
        for (size_t i = 0; i < m_fetchersNum; ++i) {
            fetchers.emplace_back(new std::thread(
                executeTasks<FetcherThreadContext, decltype(fetchUrlImpl), std::string, FetchedUrl, FetchedUrl>,
                fetcherContext, fetchUrlImpl, urlToFetch, urlToParse, urlToStore));
        }
        SIMPLE_LOG(m_logger, SL_DEBUG, m_fetchersNum << " fetchers are created.");
        for (size_t i = 0; i < m_parsersNum; ++i) {
            parsers.emplace_back(new std::thread(
                executeTasks<ParserThreadContext, decltype(parseUrlImpl), FetchedUrl, std::string>,
                parserContext, parseUrlImpl, urlToParse, parsedUrls));
        }
        SIMPLE_LOG(m_logger, SL_DEBUG, m_parsersNum << " parsers are created.");
        for (size_t i = 0; i < m_saversNum; ++i) {
            savers.emplace_back(new std::thread(
                executeTasks<SaverThreadContext, decltype(storeUrlImpl), FetchedUrl>,
                saverContext, storeUrlImpl, urlToStore));
        }
        SIMPLE_LOG(m_logger, SL_DEBUG, m_saversNum << " savers are created.");

        // Wait till all processing pools are empty
        waitForEmptyPool(*urlToFetch);
        waitForEmptyPool(*urlToParse);
        waitForEmptyPool(*urlToStore);
        
        // Setting variable workIsDone telling workers to exit
        {
            std::unique_lock<std::mutex> lck(fetcherContext->getMutex());
            fetcherContext->setWorkDone();
        }
        {
            std::unique_lock<std::mutex> lck(parserContext->getMutex());
            parserContext->setWorkDone();
        }
        {
            std::unique_lock<std::mutex> lck(saverContext->getMutex());
            saverContext->setWorkDone();
        }

        // Resume workers those were waiting non empty pools and waiting their completion
        urlToFetch->getNonEmptyCondition().notify_all();
        for (size_t i = 0; i < fetchers.size(); ++i) {
            fetchers[i]->join();
        }            
        urlToParse->getNonEmptyCondition().notify_all();
        for (size_t i = 0; i < parsers.size(); ++i) {
            parsers[i]->join();
        }
        urlToStore->getNonEmptyCondition().notify_all();
        for (size_t i = 0; i < savers.size(); ++i) {
            savers[i]->join();
        }

        // Calculating stats and swaping result pool with initial task pool
        int foundUrlsNum = 0;
        std::ostringstream urlsFound;
        while (!parsedUrls->empty()) {
            ++foundUrlsNum;
            std::string url = parsedUrls->getTask();
            urlsFound << "\"" << url << "\", ";
            urlToFetch->addTask(url);
        }
        totalUrlsFound += foundUrlsNum;
        totalUrlsSaved += saverContext->getStoredTotal();
        SIMPLE_LOG(m_logger, SL_DEBUG, urlsFound.str());
        SIMPLE_LOG(m_logger, SL_INFO, foundUrlsNum << " urls found with depth " << depth + 1 << ".");
    }    
    SIMPLE_LOG(m_logger, SL_INFO, totalUrlsFound << " urls found in total.");
    SIMPLE_LOG(m_logger, SL_INFO, totalUrlsSaved << " urls saved in total.");
}

