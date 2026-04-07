#include "TableScanNode.h"

TableScanNode::TableScanNode(const std::string& tableName)
    : m_TableName(tableName)
{
}

ExecutionNodeType TableScanNode::getType() const
{
    return ExecutionNodeType::TableScan;
}

std::string TableScanNode::toString() const
{
    return "TableScan(" + m_TableName + ")";
}

const std::string& TableScanNode::getTableName() const
{
    return m_TableName;
}