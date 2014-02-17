#include "session_factory.h"
#include "crawler_exception/crawler_exception.h"

#include <exception>
#include <sstream>


CurlSessionFactory& CurlSessionFactory::getInstance()
{
    // It is not thread-safe in Microsoft VC(both 2012 and 2013 version) compiler
    // as it doesn't support this feature of c++11.
    static CurlSessionFactory factory;
    return factory;
}


CurlSessionPtr CurlSessionFactory::createSession()
{
    return CurlSessionPtr(curl_easy_init(), curl_easy_cleanup);
}


CurlSessionFactory::CurlSessionFactory()
{
    CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
    if (result != CURLE_OK) {
        std::ostringstream os;
        os << "Can not initialize curl. CURLcode=" << result;
        throw CurlException(os.str());
    }
}


CurlSessionFactory::~CurlSessionFactory()
{
    curl_global_cleanup();
}


