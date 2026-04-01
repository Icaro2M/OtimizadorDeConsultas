#include "Query.h"

Query::Query()
{
}

void Query::addSelectField(const std::string& field)
{
	m_SelectFields.push_back(field);
}

void Query::setFromTable(const std::string& table)
{
	m_FromTable = table;
}

void Query::addJoin(const JoinClause& join)
{
	m_Joins.push_back(join);
}

void Query::addWhereCondition(const Condition& condition)
{
	m_WhereConditions.push_back(condition);
}

const std::vector<std::string>& Query::getSelectFields() const
{
	return m_SelectFields;
}

const std::string& Query::getFromTable() const
{
	return m_FromTable;
}

const std::vector<JoinClause>& Query::getJoins() const
{
	return m_Joins;
}

const std::vector<Condition>& Query::getWhereConditions() const
{
	return m_WhereConditions;
}

bool Query::hasJoins() const
{
	return !m_Joins.empty();
}

bool Query::hasWhereConditions() const
{
	return !m_WhereConditions.empty();
}