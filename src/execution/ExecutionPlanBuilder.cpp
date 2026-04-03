#include "ExecutionPlanBuilder.h"

#include "FilterNode.h"
#include "ProjectionNode.h"
#include "JoinNode.h"
#include "TableScanNode.h"

#include "../query/JoinClause.h"
#include "../query/Condition.h"
#include "../utils/StringUtils.h"

#include <algorithm>

ExecutionPlan ExecutionPlanBuilder::build(const Query& query)
{
    auto subplans = createBaseSubplans(query);

    applyLocalFilters(query, subplans);

    std::vector<Condition> pendingFilters;
    for (const Condition& cond : query.getWhereConditions())
    {
        std::vector<std::string> tables = extractReferencedTables(cond);

        if (tables.size() > 1)
        {
            pendingFilters.push_back(cond);
        }
    }

    SubplanInfo finalSubplan = buildJoinTree(query, subplans, pendingFilters);

    std::unique_ptr<ExecutionNode> root = std::move(finalSubplan.node);

    root = applyRemainingFilters(pendingFilters, std::move(root));
    root = applyProjection(query, std::move(root));

    return ExecutionPlan(std::move(root));
}

std::unordered_map<std::string, SubplanInfo>
ExecutionPlanBuilder::createBaseSubplans(const Query& query)
{
    std::unordered_map<std::string, SubplanInfo> subplans;

    subplans.emplace(
        query.getFromTable(),
        SubplanInfo(
            std::make_unique<TableScanNode>(query.getFromTable()),
            std::unordered_set<std::string>{ query.getFromTable() }
        )
    );

    for (const JoinClause& join : query.getJoins())
    {
        subplans.emplace(
            join.tableName,
            SubplanInfo(
                std::make_unique<TableScanNode>(join.tableName),
                std::unordered_set<std::string>{ join.tableName }
            )
        );
    }

    return subplans;
}

std::vector<std::string>
ExecutionPlanBuilder::extractReferencedTables(const Condition& condition) const
{
    std::vector<std::string> tables;

    auto extractTableName = [](const std::string& operand) -> std::string
        {
            std::vector<std::string> parts = StringUtils::split(operand, '.');

            if (parts.size() >= 2)
            {
                return parts[0];
            }

            return "";
        };

    std::string leftTable = extractTableName(condition.leftOperand);
    std::string rightTable = extractTableName(condition.rightOperand);

    if (!leftTable.empty())
    {
        tables.push_back(leftTable);
    }

    if (!rightTable.empty() && rightTable != leftTable)
    {
        tables.push_back(rightTable);
    }

    return tables;
}

void ExecutionPlanBuilder::applyLocalFilters(
    const Query& query,
    std::unordered_map<std::string, SubplanInfo>& subplans)
{
    for (const Condition& cond : query.getWhereConditions())
    {
        std::vector<std::string> tables = extractReferencedTables(cond);

        if (tables.size() == 1)
        {
            SubplanInfo& subplan = subplans.at(tables[0]);

            subplan.node = std::make_unique<FilterNode>(cond,std::move(subplan.node));
        }
    }
}

SubplanInfo ExecutionPlanBuilder::buildJoinTree(
    const Query& query,
    std::unordered_map<std::string, SubplanInfo>& subplans,
    std::vector<Condition>& pendingFilters)
{
    SubplanInfo currentSubplan = std::move(subplans.at(query.getFromTable()));

    for (const JoinClause& join : query.getJoins())
    {
        SubplanInfo rightSubplan = std::move(subplans.at(join.tableName));

        std::unique_ptr<ExecutionNode> joinedNode = std::make_unique<JoinNode>(
            join.onCondition,
            std::move(currentSubplan.node),
            std::move(rightSubplan.node)
        );

        std::unordered_set<std::string> joinedTables = currentSubplan.tables;
        joinedTables.insert(rightSubplan.tables.begin(), rightSubplan.tables.end());

        for (auto it = pendingFilters.begin(); it != pendingFilters.end();)
        {
            std::vector<std::string> referencedTables = extractReferencedTables(*it);

            bool canApplyHere = true;
            for (const std::string& table : referencedTables)
            {
                if (joinedTables.find(table) == joinedTables.end())
                {
                    canApplyHere = false;
                    break;
                }
            }

            if (canApplyHere)
            {
                joinedNode = std::make_unique<FilterNode>(*it, std::move(joinedNode));

                it = pendingFilters.erase(it);
            }
            else
            {
                ++it;
            }
        }

        currentSubplan = SubplanInfo(std::move(joinedNode), joinedTables);
    }

    return currentSubplan;
}

std::unique_ptr<ExecutionNode>
ExecutionPlanBuilder::applyRemainingFilters(
    const std::vector<Condition>& pendingFilters,
    std::unique_ptr<ExecutionNode> currentNode)
{
    for (const Condition& cond : pendingFilters)
    {
        currentNode = std::make_unique<FilterNode>(cond, std::move(currentNode));
    }

    return currentNode;
}

std::unique_ptr<ExecutionNode>
ExecutionPlanBuilder::applyProjection(
    const Query& query,
    std::unique_ptr<ExecutionNode> currentNode)
{
    return std::make_unique<ProjectionNode>(query.getSelectFields(), std::move(currentNode));
}