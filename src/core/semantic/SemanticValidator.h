#pragma once

#include "../metadata/MetadataCatalog.h"
#include "../query/Query.h"
#include "../query/Condition.h"

#include <string>
#include <vector>

class SemanticValidator
{
private:
    const MetadataCatalog& m_MetadataCatalog;

private:
    void validateFromTable(const Query& query) const;
    void validateJoins(const Query& query) const;
    void validateSelectFields(const Query& query) const;
    void validateWhereConditions(const Query& query) const;

    void validateCondition(const Condition& condition, const Query& query) const;
    void validateOperand(const Operand& operand, const Query& query) const;
    void validateConditionTypes(const Condition& condition, const Query& query) const;

    ColumnType resolveOperandType(const Operand& operand, const Query& query) const;
    ColumnType resolveIdentifierType(const std::string& identifier, const Query& query) const;

    bool isNumericType(ColumnType type) const;
    bool isComparableWithOrdering(ColumnType type) const;
    bool isQualifiedField(const std::string& field) const;
    bool areCompatibleTypes(ColumnType leftType, ColumnType rightType) const;

    void validateQualifiedField(const std::string& field) const;
    void validateUnqualifiedField(const std::string& field, const Query& query) const;

    std::vector<std::string> getTablesInQuery(const Query& query) const;

public:
    SemanticValidator(const MetadataCatalog& metadataCatalog);

    void validate(const Query& query) const;
};