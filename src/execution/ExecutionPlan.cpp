#include "ExecutionPlan.h"

ExecutionPlan::ExecutionPlan(std::unique_ptr<ExecutionNode> root)
	: m_Root(std::move(root))
{
}

const ExecutionNode* ExecutionPlan::getRoot() const
{
	return m_Root.get();
}

std::unique_ptr<ExecutionNode> ExecutionPlan::releaseRoot()
{
	return std::move(m_Root);
}
