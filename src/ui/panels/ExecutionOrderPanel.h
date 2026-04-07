#pragma once

#include <string>
#include <vector>

class ExecutionOrderPanel
{
public:
    void render(const std::vector<std::string>& executionOrder) const;
};