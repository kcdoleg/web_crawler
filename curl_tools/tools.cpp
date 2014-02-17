#include "tools.h"
#include "session_factory.h"
#include "crawler_exception/crawler_exception.h"
#include <curl/curl.h>

#include <string>
#include <iostream>
#include <sstream>
#include <cstring>

static size_t writeDataCallback(void *ptr, size_t size, size_t nmemb, void *stringPtr)
{
    try {
        std::string& str = *static_cast<std::string*>(stringPtr);
        size_t currentLen = str.length();
        size_t additionalLen = size * nmemb;
        str.resize(currentLen + additionalLen);
        memcpy(&str[currentLen], ptr, additionalLen);
        return additionalLen;
    } catch (...) { // Can not throw any exception in c-runtime callbacks
        std::cerr << "Can not write data to the string in curl callback." << std::endl;
        return 0;
    }
}

void fetchUrl(const std::string& url, std::string* result) {
    CurlSessionPtr session = CurlSessionFactory::getInstance().createSession();
    if (session) {
        curl_easy_setopt(session.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(session.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(session.get(), CURLOPT_WRITEFUNCTION, writeDataCallback);
        curl_easy_setopt(session.get(), CURLOPT_WRITEDATA, result);
        CURLcode res = curl_easy_perform(session.get());
        if (res != CURLE_OK) {
            std::ostringstream os;
            os << "CurlTools: can not perform request to \"" << url << "\". Error code: " << res;
            throw CurlException(os.str());
        }    
    }
}
