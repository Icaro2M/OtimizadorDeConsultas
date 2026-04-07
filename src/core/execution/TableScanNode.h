#pragma once

#include "ExecutionNode.h"
#include <string>

class TableScanNode : public ExecutionNode
{
private:
    std::string m_TableName;

public:
    TableScanNode(const std::string& tableName);

    ExecutionNodeType getType() const override;
    std::string toString() const override;

    const std::string& getTableName() const;
};