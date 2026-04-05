#pragma once

#include "../execution/ExecutionPlan.h"
#include "../execution/ExecutionNode.h"
#include "../query/Condition.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

class Optimizer
{
public:
    ExecutionPlan optimize(ExecutionPlan plan);

private:
    std::unique_ptr<ExecutionNode> optimizeNode(std::unique_ptr<ExecutionNode> node);

    std::unique_ptr<ExecutionNode> optimizeFilterNode(std::unique_ptr<ExecutionNode> node);
    std::unique_ptr<ExecutionNode> tryPushDownFilter(
        const Condition& condition,
        std::unique_ptr<ExecutionNode> child
    );

    std::unique_ptr<ExecutionNode> tryPushDownProjection(
        const std::vector<std::string>& requiredColumns,
        std::unique_ptr<ExecutionNode> child
    );

    std::unordered_set<std::string> extractReferencedTables(const Condition& condition) const;
    std::vector<std::string> extractReferencedColumns(const Condition& condition) const;
    std::unordered_set<std::string> collectTables(const ExecutionNode* node) const;

    int computeRestrictionScore(const ExecutionNode* node) const;

    void collectJoinOperands(
        std::unique_ptr<ExecutionNode> node,
        std::vector<std::unique_ptr<ExecutionNode>>& operands
    );

    void collectJoinConditions(
        const ExecutionNode* node,
        std::vector<Condition>& conditions
    ) const;

    std::unique_ptr<ExecutionNode> rebuildJoinTree(
        std::vector<std::unique_ptr<ExecutionNode>>& operands,
        const std::vector<Condition>& conditions
    );

    std::unique_ptr<ExecutionNode> reorderJoinTree(std::unique_ptr<ExecutionNode> node);
};