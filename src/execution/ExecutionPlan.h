#pragma once

#include "ExecutionNode.h"

#include <memory>

class ExecutionPlan
{
private:
	std::unique_ptr<ExecutionNode> m_Root;

public:
	ExecutionPlan(std::unique_ptr<ExecutionNode> root);
	const ExecutionNode* getRoot() const;
};