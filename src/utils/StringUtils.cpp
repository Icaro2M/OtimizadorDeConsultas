#include "StringUtils.h"

#include <cctype>
#include <algorithm>
#include <sstream>
#include <numeric>

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

std::string StringUtils::vecToString(const std::vector<std::string>& vector)
{
    std::string vectorStr = std::accumulate(vector.begin(), vector.end(), std::string(""),
        [](const std::string& a, const std::string& b)
        {
            return a.empty() ? b : a + ", " + b;
        });

    return vectorStr;
}