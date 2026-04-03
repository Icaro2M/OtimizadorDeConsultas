#pragma once

#include "ExecutionNode.h"
#include "../query/Condition.h"

#include <memory>
#include <string>

class JoinNode : public ExecutionNode
{
private:
	Condition m_JoinCondition;
	std::unique_ptr<ExecutionNode> m_LeftChild;
	std::unique_ptr<ExecutionNode> m_RightChild;

public:
	JoinNode(const Condition& joinCondition,
		std::unique_ptr<ExecutionNode> leftChild,
		std::unique_ptr<ExecutionNode> rightChild);

	ExecutionNodeType getType() const override;
	std::string toString() const override;

	const Condition& getJoinCondition() const;
	const ExecutionNode* getLeftChild() const;
	const ExecutionNode* getRightChild() const;
};