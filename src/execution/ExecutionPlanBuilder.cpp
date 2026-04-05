#include "ExecutionPlanBuilder.h"

#include "FilterNode.h"
#include "ProjectionNode.h"
#include "JoinNode.h"
#include "TableScanNode.h"

#include "../query/JoinClause.h"
#include "../query/Condition.h"

ExecutionPlan ExecutionPlanBuilder::build(const Query& query)
{
    std::unique_ptr<ExecutionNode> root = buildBasePlan(query);
    root = applyJoins(query, std::move(root));
    root = applyWhereFilters(query, std::move(root));
    root = applyProjection(query, std::move(root));

    return ExecutionPlan(std::move(root));
}

std::unique_ptr<ExecutionNode> ExecutionPlanBuilder::buildBasePlan(const Query& query)
{
    return std::make_unique<TableScanNode>(query.getFromTable());
}

std::unique_ptr<ExecutionNode> ExecutionPlanBuilder::applyJoins(
    const Query& query,
    std::unique_ptr<ExecutionNode> currentNode
)
{
    for (const JoinClause& join : query.getJoins())
    {
        std::unique_ptr<ExecutionNode> rightNode =
            std::make_unique<TableScanNode>(join.tableName);

        currentNode = std::make_unique<JoinNode>(
            join.onCondition,
            std::move(currentNode),
            std::move(rightNode)
        );
    }

    return currentNode;
}

std::unique_ptr<ExecutionNode> ExecutionPlanBuilder::applyWhereFilters(
    const Query& query,
    std::unique_ptr<ExecutionNode> currentNode
)
{
    for (const Condition& condition : query.getWhereConditions())
    {
        currentNode = std::make_unique<FilterNode>(condition, std::move(currentNode));
    }

    return currentNode;
}

std::unique_ptr<ExecutionNode> ExecutionPlanBuilder::applyProjection(
    const Query& query,
    std::unique_ptr<ExecutionNode> currentNode
)
{
    return std::make_unique<ProjectionNode>(query.getSelectFields(), std::move(currentNode));
}