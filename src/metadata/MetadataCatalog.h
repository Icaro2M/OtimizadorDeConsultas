#pragma once

#include <string>
#include <vector>
#include <map>

enum class ColumnType
{
    Integer,
    Decimal,
    String,
    DateTime
};

struct ColumnMetadata
{
    std::string name;
    ColumnType type;
};

struct TableMetadata
{
    std::string name;
    std::vector<ColumnMetadata> columns;
};

class MetadataCatalog
{
private:
    std::map<std::string, TableMetadata> m_tables;

public:
    MetadataCatalog();

    bool tableExists(const std::string& tableName) const;
    bool columnExists(const std::string& tableName, const std::string& columnName) const;
    ColumnType getColumnType(const std::string& tableName, const std::string& columnName) const;
};