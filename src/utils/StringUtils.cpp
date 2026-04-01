#include "StringUtils.h"

#include <cctype>
#include <algorithm>
#include <sstream>

std::string StringUtils::toLowerCopy(const std::string& text)
{
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}


std::vector<std::string> StringUtils::split(const std::string text, char divisor)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(text);

    while (std::getline(tokenStream, token, divisor))
    {
        tokens.push_back(token);
    }

    return tokens;
}