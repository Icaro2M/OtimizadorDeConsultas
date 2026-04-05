#pragma once
#include <string>
#include <vector>

namespace StringUtils
{
    std::string toLowerCopy(const std::string& text);

    std::vector<std::string> split(const std::string text, char divisor);

    std::string vecToString(const std::vector<std::string>& vector);

    std::vector<std::string> vecIntersection(
        const std::vector<std::string>& vectorA,
        const std::vector<std::string>& vectorB
    );

    std::vector<std::string> vecUnion(
        const std::vector<std::string>& vectorA,
        const std::vector<std::string>& vectorB
    );
    std::vector<std::string> removeDuplicates(const std::vector<std::string>& values);
}