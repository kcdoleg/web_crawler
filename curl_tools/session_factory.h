#pragma once

#include <curl/curl.h>

#include <memory>
#include <functional>


typedef std::unique_ptr<CURL, void(*)(CURL*)> CurlSessionPtr;


/* Singleton class for creating sessions.
 * We need it because we have to call curl_global_init() at some point of program
 * and curl_global_init() is not thread-safe.
 */
class CurlSessionFactory
{
public:
    static CurlSessionFactory& getInstance();
    CurlSessionPtr createSession();

private:
    CurlSessionFactory();
    ~CurlSessionFactory();
    CurlSessionFactory(const CurlSessionFactory&) = delete;
    CurlSessionFactory& operator=(const CurlSessionFactory&) = delete;
};


