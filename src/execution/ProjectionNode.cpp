#include "ProjectionNode.h"
#include "../utils/StringUtils.h"

ProjectionNode::ProjectionNode(
    const std::vector<std::string>& selectedFields,
    std::unique_ptr<ExecutionNode> child
)
    : m_SelectedFields(selectedFields), m_Child(std::move(child))
{
}

ExecutionNodeType ProjectionNode::getType() const
{
    return ExecutionNodeType::Projection;
}

std::string ProjectionNode::toString() const
{
    return "Projection(" + StringUtils::vecToString(m_SelectedFields) + ")";
}

const std::vector<std::string>& ProjectionNode::getSelectedFields() const
{
    return m_SelectedFields;
}

const ExecutionNode* ProjectionNode::getChild() const
{
    return m_Child.get();
}

std::unique_ptr<ExecutionNode> ProjectionNode::releaseChild()
{
    return std::move(m_Child);
}