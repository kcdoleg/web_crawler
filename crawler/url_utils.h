#pragma once

#include <string>
#include <vector>

// Parse new urls from text
void parseUrls(const std::string& text, std::vector<std::string>* result,
    const std::string& filter = "");


// Calculate md5 sum for document to avoid multiple visiting of one url.
std::string calcMd5(const std::string& string);
