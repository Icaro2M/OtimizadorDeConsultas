#pragma once

#include <string>
#include <vector>
#include <map>

struct TableMetadata {
    std::string name;
    std::vector<std::string> columns;
};

class MetadataCatalog {
private:
    std::map<std::string, TableMetadata> m_tables;

public:
    MetadataCatalog();
    bool tableExists(const std::string& tableName) const;
    bool columnExists(const std::string& tableName, const std::string& columnName) const;
};