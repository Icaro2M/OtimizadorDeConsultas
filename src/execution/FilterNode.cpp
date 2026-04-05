#include "FilterNode.h"

FilterNode::FilterNode(const Condition& condition, std::unique_ptr<ExecutionNode> child)
    : m_Condition(condition), m_Child(std::move(child))
{
}

ExecutionNodeType FilterNode::getType() const
{
    return ExecutionNodeType::Filter;
}

std::string FilterNode::toString() const
{
    return "Filter(" + m_Condition.leftOperand + " " + m_Condition.op + " " + m_Condition.rightOperand + ")";
}

const Condition& FilterNode::getCondition() const
{
    return m_Condition;
}

const ExecutionNode* FilterNode::getChild() const
{
    return m_Child.get();
}

std::unique_ptr<ExecutionNode> FilterNode::releaseChild()
{
    return std::move(m_Child);
}