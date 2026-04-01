#pragma once

#include "Condition.h"
#include "JoinClause.h"

#include <string>
#include <vector>

class Query
{
private:
    std::vector<std::string> m_SelectFields;
    std::string m_FromTable;
    std::vector<JoinClause> m_Joins;
    std::vector<Condition> m_WhereConditions;

public:
    Query();

    void addSelectField(const std::string& field);
    void setFromTable(const std::string& table);
    void addJoin(const JoinClause& join);
    void addWhereCondition(const Condition& condition);

    const std::vector<std::string>& getSelectFields() const;
    const std::string& getFromTable() const;
    const std::vector<JoinClause>& getJoins() const;
    const std::vector<Condition>& getWhereConditions() const;

    bool hasJoins() const;
    bool hasWhereConditions() const;
};