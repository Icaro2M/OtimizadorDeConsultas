#include "SemanticValidator.h"

#include "../utils/StringUtils.h"

#include <cctype>
#include <stdexcept>

SemanticValidator::SemanticValidator(const MetadataCatalog& metadataCatalog)
    : m_MetadataCatalog(metadataCatalog)
{
}

void SemanticValidator::validate(const Query& query) const
{
    validateFromTable(query);
    validateJoins(query);
    validateSelectFields(query);
    validateWhereConditions(query);
}

void SemanticValidator::validateFromTable(const Query& query) const
{
    const std::string& table = query.getFromTable();

    if (!m_MetadataCatalog.tableExists(table))
    {
        throw std::invalid_argument("a tabela '" + table + "' nao existe no banco.");
    }
}

void SemanticValidator::validateJoins(const Query& query) const
{
    const std::vector<JoinClause>& joins = query.getJoins();

    for (const JoinClause& join : joins)
    {
        if (!m_MetadataCatalog.tableExists(join.tableName))
        {
            throw std::invalid_argument("a tabela '" + join.tableName + "' nao existe no banco.");
        }

        validateCondition(join.onCondition, query);
    }
}

void SemanticValidator::validateSelectFields(const Query& query) const
{
    const std::vector<std::string>& selectFields = query.getSelectFields();

    for (const std::string& field : selectFields)
    {
        if (isQualifiedField(field))
        {
            validateQualifiedField(field);
        }
        else
        {
            validateUnqualifiedField(field, query);
        }
    }
}

void SemanticValidator::validateWhereConditions(const Query& query) const
{
    const std::vector<Condition>& whereConditions = query.getWhereConditions();

    for (const Condition& condition : whereConditions)
    {
        validateCondition(condition, query);
    }
}

void SemanticValidator::validateCondition(const Condition& condition, const Query& query) const
{
    validateOperand(condition.leftOperand, query);
    validateOperand(condition.rightOperand, query);
}

void SemanticValidator::validateOperand(const std::string& operand, const Query& query) const
{
    if (operand.empty())
    {
        throw std::invalid_argument("operando vazio em condicao.");
    }

    if (isNumeric(operand))
    {
        return;
    }

    if (operand.size() >= 2 && operand.front() == '\'' && operand.back() == '\'')
    {
        return;
    }

    if (isQualifiedField(operand))
    {
        validateQualifiedField(operand);
    }
    else
    {
        validateUnqualifiedField(operand, query);
    }
}

bool SemanticValidator::isNumeric(const std::string& text) const
{
    if (text.empty())
        return false;

    size_t start = 0;

    if (text[0] == '+' || text[0] == '-')
    {
        if (text.size() == 1)
            return false;

        start = 1;
    }

    bool hasDigit = false;
    bool hasDot = false;

    for (size_t i = start; i < text.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(text[i]);

        if (std::isdigit(c))
        {
            hasDigit = true;
            continue;
        }

        if (text[i] == '.' && !hasDot)
        {
            hasDot = true;
            continue;
        }

        return false;
    }

    return hasDigit;
}

bool SemanticValidator::isQualifiedField(const std::string& field) const
{
    return field.find('.') != std::string::npos;
}

void SemanticValidator::validateQualifiedField(const std::string& field) const
{
    std::vector<std::string> parts = StringUtils::split(field, '.');

    if (parts.size() != 2)
    {
        throw std::invalid_argument("o campo '" + field + "' nao esta no formato tabela.coluna.");
    }

    const std::string& tableName = parts[0];
    const std::string& columnName = parts[1];

    if (!m_MetadataCatalog.tableExists(tableName))
    {
        throw std::invalid_argument("a tabela '" + tableName + "' nao existe no banco.");
    }

    if (!m_MetadataCatalog.columnExists(tableName, columnName))
    {
        throw std::invalid_argument("a coluna '" + columnName + "' nao existe na tabela '" + tableName + "'.");
    }
}

void SemanticValidator::validateUnqualifiedField(const std::string& field, const Query& query) const
{
    std::vector<std::string> tables = getTablesInQuery(query);

    int foundCount = 0;

    for (const std::string& table : tables)
    {
        if (m_MetadataCatalog.columnExists(table, field))
        {
            foundCount++;
        }
    }

    if (foundCount == 0)
    {
        throw std::invalid_argument("a coluna '" + field + "' nao existe em nenhuma tabela da consulta.");
    }

    if (foundCount > 1)
    {
        throw std::invalid_argument("a coluna '" + field + "' esta ambigua na consulta.");
    }
}

std::vector<std::string> SemanticValidator::getTablesInQuery(const Query& query) const
{
    std::vector<std::string> tables;

    tables.push_back(query.getFromTable());

    const std::vector<JoinClause>& joins = query.getJoins();
    for (const JoinClause& join : joins)
    {
        tables.push_back(join.tableName);
    }

    return tables;
}