#include "StringUtils.h"

#include <cctype>
#include <algorithm>
#include <sstream>
#include <numeric>
#include <unordered_set>

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

std::vector<std::string> StringUtils::vecIntersection(
    const std::vector<std::string>& vectorA,
    const std::vector<std::string>& vectorB
)
{
    std::unordered_set<std::string> s(vectorA.begin(), vectorA.end());
    std::vector<std::string> intersection;

    intersection.reserve(std::min(vectorA.size(), vectorB.size()));

    for (const auto& item : vectorB) {
        if (s.erase(item)) {
            intersection.push_back(item);
        }
    }
    return intersection;
}

std::vector<std::string> StringUtils::vecUnion(
    const std::vector<std::string>& vectorA,
    const std::vector<std::string>& vectorB
) {
    std::unordered_set<std::string> s(vectorA.begin(), vectorA.end());

    for (const auto& item : vectorB) {
        s.insert(item);
    }

    return std::vector<std::string>(s.begin(), s.end());
}

std::vector<std::string> StringUtils::removeDuplicates(const std::vector<std::string>& values)
{
    std::vector<std::string> result;

    for (const std::string& value : values)
    {
        bool alreadyExists = false;

        for (const std::string& existing : result)
        {
            if (existing == value)
            {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists)
        {
            result.push_back(value);
        }
    }

    return result;
}