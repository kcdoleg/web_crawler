#include "crawler_exception.h"


const char* CrawlerException::what() const throw()
{
    try {
        return m_msg.c_str();
    } catch (...) {
        return "Unexpected behaviour in c_str() function when extracting exception message.";
    } 
}


CrawlerException::CrawlerException(const std::string& msg)
    : m_msg(msg)
{
    
}


CrawlerException::~CrawlerException() throw()
{

}


CurlException::CurlException(const std::string& msg)
    : CrawlerException(msg)
{

}


CrawlingParamsError::CrawlingParamsError(const std::string& msg)
    : CrawlerException(msg)
{

}

