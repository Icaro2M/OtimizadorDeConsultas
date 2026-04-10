#include "SemanticValidator.h"

#include "../../utils/StringUtils.h"

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
    validateConditionTypes(condition, query);
}

void SemanticValidator::validateOperand(const Operand& operand, const Query& query) const
{
    if (operand.value.empty())
    {
        throw std::invalid_argument("operando vazio em condicao.");
    }

    if (operand.type == OperandType::Number)
    {
        return;
    }

    if (operand.type == OperandType::StringLiteral)
    {
        return;
    }

    if (operand.type == OperandType::Identifier)
    {
        if (isQualifiedField(operand.value))
        {
            validateQualifiedField(operand.value);
        }
        else
        {
            validateUnqualifiedField(operand.value, query);
        }

        return;
    }

    throw std::invalid_argument("tipo de operando invalido em condicao.");
}

void SemanticValidator::validateConditionTypes(const Condition& condition, const Query& query) const
{
    ColumnType leftType = resolveOperandType(condition.leftOperand, query);
    ColumnType rightType = resolveOperandType(condition.rightOperand, query);

    if (condition.op == "=" || condition.op == "<>")
    {
        if (!areCompatibleTypes(leftType,rightType))
        {
            throw std::invalid_argument(
                "tipos incompativeis na condicao: " +
                condition.leftOperand.value + " " +
                condition.op + " " +
                condition.rightOperand.value
            );
        }

        return;
    }

    if (condition.op == ">" || condition.op == "<" || condition.op == ">=" || condition.op == "<=")
    {
        if (!isComparableWithOrdering(leftType) || !isComparableWithOrdering(rightType))
        {
            throw std::invalid_argument(
                "operacao relacional invalida na condicao: " +
                condition.leftOperand.value + " " +
                condition.op + " " +
                condition.rightOperand.value
            );
        }

        if (!areCompatibleTypes(leftType, rightType))
        {
            throw std::invalid_argument(
                "tipos incompativeis na condicao: " +
                condition.leftOperand.value + " " +
                condition.op + " " +
                condition.rightOperand.value
            );
        }

        return;
    }

    throw std::invalid_argument("operador relacional invalido na condicao.");
}

ColumnType SemanticValidator::resolveOperandType(const Operand& operand, const Query& query) const
{
    if (operand.type == OperandType::Number)
    {
        if (operand.value.find('.') != std::string::npos)
            return ColumnType::Decimal;

        return ColumnType::Integer;
    }

    if (operand.type == OperandType::StringLiteral)
    {
        return ColumnType::String;
    }

    if (operand.type == OperandType::Identifier)
    {
        return resolveIdentifierType(operand.value, query);
    }

    throw std::invalid_argument("nao foi possivel resolver o tipo do operando.");
}

ColumnType SemanticValidator::resolveIdentifierType(const std::string& identifier, const Query& query) const
{
    if (isQualifiedField(identifier))
    {
        std::vector<std::string> parts = StringUtils::split(identifier, '.');
        return m_MetadataCatalog.getColumnType(parts[0], parts[1]);
    }

    std::vector<std::string> tables = getTablesInQuery(query);
    std::string resolvedTable;
    int foundCount = 0;

    for (const std::string& table : tables)
    {
        if (m_MetadataCatalog.columnExists(table, identifier))
        {
            resolvedTable = table;
            foundCount++;
        }
    }

    if (foundCount == 0)
    {
        throw std::invalid_argument("a coluna '" + identifier + "' nao existe em nenhuma tabela da consulta.");
    }

    if (foundCount > 1)
    {
        throw std::invalid_argument("a coluna '" + identifier + "' esta ambigua na consulta.");
    }

    return m_MetadataCatalog.getColumnType(resolvedTable, identifier);
}

bool SemanticValidator::isNumericType(ColumnType type) const
{
    return type == ColumnType::Integer || type == ColumnType::Decimal;
}

bool SemanticValidator::isComparableWithOrdering(ColumnType type) const
{
    return isNumericType(type) || type == ColumnType::DateTime;
}

bool SemanticValidator::isQualifiedField(const std::string& field) const
{
    return field.find('.') != std::string::npos;
}

bool SemanticValidator::areCompatibleTypes(ColumnType leftType, ColumnType rightType) const
{
    if (isNumericType(leftType) && isNumericType(rightType))
    {
        return true;
    }

    return leftType == rightType;
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