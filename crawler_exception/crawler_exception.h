#pragma once

#include <exception>
#include <string>


class CrawlerException: public std::exception
{
public:
    CrawlerException(const std::string& msg);
    virtual ~CrawlerException() throw();
    virtual const char* what() const throw();

private:
    std::string m_msg;
};


class CurlException: public CrawlerException
{
public:
    CurlException(const std::string& msg);
};


class CrawlingParamsError: public CrawlerException
{
public:
    CrawlingParamsError(const std::string& msg);
};
