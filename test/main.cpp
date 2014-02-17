#include "crawler/crawler.h"
#include "logger/logger.h"
#include "crawler/url_utils.h"

#include <iostream>


int main(int argc, char* argv[]) 
{
    Logger logger(std::cerr, SL_INFO);
    Crawler crawler(logger);
    crawler.initVisitingParams(argv[1], atoi(argv[2]), argv[3]);
    crawler.run();

    return 0;
}
