#pragma once
#include <string>
#include <vector>

namespace StringUtils
{
    std::string toLowerCopy(const std::string& text);
    std::vector<std::string> split(const std::string text, char divisor);
    std::string vecToString(const std::vector<std::string>& vector);
}