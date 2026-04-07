#pragma once

#include "ExecutionNode.h"
#include <memory>
#include <string>
#include <vector>

class ProjectionNode : public ExecutionNode
{
private:
    std::vector<std::string> m_SelectedFields;
    std::unique_ptr<ExecutionNode> m_Child;

public:
    ProjectionNode(
        const std::vector<std::string>& selectedFields,
        std::unique_ptr<ExecutionNode> child
    );

    ExecutionNodeType getType() const override;
    std::string toString() const override;

    const std::vector<std::string>& getSelectedFields() const;
    const ExecutionNode* getChild() const;
    std::unique_ptr<ExecutionNode> releaseChild();
};