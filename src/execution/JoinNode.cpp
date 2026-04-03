#include "JoinNode.h"



JoinNode::JoinNode(const Condition& joinCondition,
	std::unique_ptr<ExecutionNode> leftChild,
	std::unique_ptr<ExecutionNode> rightChild)
	: m_JoinCondition(joinCondition),
	m_LeftChild(std::move(leftChild)),
	m_RightChild(std::move(rightChild))
{
}

ExecutionNodeType JoinNode::getType() const
{
	return ExecutionNodeType::Join;
}

std::string JoinNode::toString() const
{
	return "Join(" + m_JoinCondition.leftOperand + " " + m_JoinCondition.op + " " + m_JoinCondition.rightOperand + ")";
}

const Condition& JoinNode::getJoinCondition() const
{
	return m_JoinCondition;
}

const ExecutionNode* JoinNode::getLeftChild() const
{
	return m_LeftChild.get();
}

const ExecutionNode* JoinNode::getRightChild() const
{
	return m_RightChild.get();
}