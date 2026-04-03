#pragma once

#include "../query/Condition.h"
#include "ExecutionNode.h"

#include <string>
#include <memory>

class FilterNode : public ExecutionNode
{
private:
    Condition m_Condition;
    std::unique_ptr<ExecutionNode> m_Child;

public:
    FilterNode(const Condition& condition, std::unique_ptr<ExecutionNode> child);

    ExecutionNodeType getType() const override;
    std::string toString() const override;

    const Condition& getCondition() const;
    const ExecutionNode* getChild() const;
};