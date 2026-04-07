#pragma once

#include "ExecutionPlan.h"
#include "../query/Query.h"
#include "../query/Condition.h"
#include <memory>

class ExecutionPlanBuilder
{
public:
    ExecutionPlan build(const Query& query);

private:
    std::unique_ptr<ExecutionNode> buildBasePlan(const Query& query);

    std::unique_ptr<ExecutionNode> applyJoins(
        const Query& query,
        std::unique_ptr<ExecutionNode> currentNode
    );

    std::unique_ptr<ExecutionNode> applyWhereFilters(
        const Query& query,
        std::unique_ptr<ExecutionNode> currentNode
    );

    std::unique_ptr<ExecutionNode> applyProjection(
        const Query& query,
        std::unique_ptr<ExecutionNode> currentNode
    );
};