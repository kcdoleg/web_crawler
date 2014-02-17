#include "url_utils.h"

#include <openssl/md5.h>

#include <sstream>


void parseUrls(const std::string& text, std::vector<std::string>* result,
    const std::string& filter /*= ""*/)
{
    size_t start = 0;
    size_t end = text.length();
    while (start < end) {
        size_t first = text.find("href=\"", start);
        if (first == std::string::npos) {
            break;
        }
        first += 6;
        size_t last = text.find("\"", first);
        if (last == std::string::npos) {
            break;
        }
        std::string next(text.begin() + first, text.begin() + last);
        if (next.find('.') != std::string::npos &&
            (filter.empty() || next.find(filter) != std::string::npos))
        {
            result->push_back(next);
        }
        start = last + 1;
    }
}


std::string calcMd5(const std::string& string)
{
    MD5_CTX md5handler;
    unsigned char md5digest[MD5_DIGEST_LENGTH];
                     
    MD5_Init(&md5handler);
    MD5_Update(&md5handler, static_cast<const void*>(&string[0]), string.length());
    MD5_Final(md5digest, &md5handler);
                                    
    std::ostringstream os;    
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        os << std::hex << static_cast<unsigned int>(md5digest[i]);
    };
                                                               
    return os.str();
}

