#pragma once

#include "ExecutionPlan.h"
#include "ExecutionNode.h"
#include "SubplanInfo.h"

#include "../query/Query.h"
#include "../query/Condition.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class ExecutionPlanBuilder
{
public:
    ExecutionPlan build(const Query& query);

private:
    std::unordered_map<std::string, SubplanInfo> createBaseSubplans(const Query& query);

    std::vector<std::string> extractReferencedTables(const Condition& condition) const;

    void applyLocalFilters(
        const Query& query,
        std::unordered_map<std::string, SubplanInfo>& subplans
    );

    SubplanInfo buildJoinTree(
        const Query& query,
        std::unordered_map<std::string, SubplanInfo>& subplans,
        std::vector<Condition>& pendingFilters
    );

    std::unique_ptr<ExecutionNode> applyRemainingFilters(
        const std::vector<Condition>& pendingFilters,
        std::unique_ptr<ExecutionNode> currentNode
    );

    std::unique_ptr<ExecutionNode> applyProjection(
        const Query& query,
        std::unique_ptr<ExecutionNode> currentNode
    );
};