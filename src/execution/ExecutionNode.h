#pragma once

#include <string>

enum class ExecutionNodeType
{
    TableScan,
    Filter,
    Join,
    Projection
};

class ExecutionNode
{
public:
    virtual ~ExecutionNode();

    virtual ExecutionNodeType getType() const = 0;
    virtual std::string toString() const = 0;
};