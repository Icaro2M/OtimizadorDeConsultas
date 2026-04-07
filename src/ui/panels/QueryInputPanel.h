#pragma once

#include <array>
#include <string>

class QueryInputPanel
{
public:
    QueryInputPanel();

    bool render();
    std::string getSql() const;
    void setSql(const std::string& sql);

private:
    static constexpr std::size_t BUFFER_SIZE = 4096;
    std::array<char, BUFFER_SIZE> m_Buffer;
};