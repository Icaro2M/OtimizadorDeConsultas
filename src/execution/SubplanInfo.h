#pragma once

#pragma once

#include "ExecutionNode.h"

#include <memory>
#include <string>
#include <unordered_set>

struct SubplanInfo
{
    std::unique_ptr<ExecutionNode> node;
    std::unordered_set<std::string> tables;

    SubplanInfo() = default;

    SubplanInfo(
        std::unique_ptr<ExecutionNode> node,
        const std::unordered_set<std::string>& tables)
        : node(std::move(node)), tables(tables)
    {
    }
};