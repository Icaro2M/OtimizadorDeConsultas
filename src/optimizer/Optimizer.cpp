#include "Optimizer.h"

#include "../execution/FilterNode.h"
#include "../execution/JoinNode.h"
#include "../execution/ProjectionNode.h"
#include "../execution/TableScanNode.h"
#include "../utils/StringUtils.h"

#include <utility>
#include <algorithm>

ExecutionPlan Optimizer::optimize(ExecutionPlan plan)
{
    std::unique_ptr<ExecutionNode> root = plan.releaseRoot();
    root = optimizeNode(std::move(root));
    return ExecutionPlan(std::move(root));
}

std::unique_ptr<ExecutionNode> Optimizer::optimizeNode(std::unique_ptr<ExecutionNode> node)
{
    if (!node)
    {
        return nullptr;
    }

    ExecutionNodeType nodeType = node->getType();

    if (nodeType == ExecutionNodeType::TableScan)
    {
        return node;
    }

    if (nodeType == ExecutionNodeType::Projection)
    {
        ProjectionNode* projectionNode = dynamic_cast<ProjectionNode*>(node.get());

        std::vector<std::string> finalColumns = projectionNode->getSelectedFields();

        std::unique_ptr<ExecutionNode> optimizedChild =
            optimizeNode(projectionNode->releaseChild());

        std::unique_ptr<ExecutionNode> pushedChild =
            tryPushDownProjection(finalColumns, std::move(optimizedChild));

        if (pushedChild && pushedChild->getType() == ExecutionNodeType::Projection)
        {
            ProjectionNode* childProjection =
                dynamic_cast<ProjectionNode*>(pushedChild.get());

            if (childProjection->getSelectedFields() == finalColumns)
            {
                std::unique_ptr<ExecutionNode> grandChild =
                    childProjection->releaseChild();

                return std::make_unique<ProjectionNode>(
                    finalColumns,
                    std::move(grandChild)
                );
            }
        }

        return std::make_unique<ProjectionNode>(
            finalColumns,
            std::move(pushedChild)
        );
    }

    if (nodeType == ExecutionNodeType::Join)
    {
        JoinNode* joinNode = dynamic_cast<JoinNode*>(node.get());

        Condition joinCondition = joinNode->getJoinCondition();

        std::unique_ptr<ExecutionNode> optimizedLeft =
            optimizeNode(joinNode->releaseLeftChild());

        std::unique_ptr<ExecutionNode> optimizedRight =
            optimizeNode(joinNode->releaseRightChild());

        std::unique_ptr<ExecutionNode> rebuiltJoin =
            std::make_unique<JoinNode>(
                joinCondition,
                std::move(optimizedLeft),
                std::move(optimizedRight)
            );

        return reorderJoinTree(std::move(rebuiltJoin));
    }

    if (nodeType == ExecutionNodeType::Filter)
    {
        return optimizeFilterNode(std::move(node));
    }

    return node;
}

std::unique_ptr<ExecutionNode> Optimizer::optimizeFilterNode(std::unique_ptr<ExecutionNode> node)
{
    FilterNode* filterNode = dynamic_cast<FilterNode*>(node.get());

    Condition condition = filterNode->getCondition();
    std::unique_ptr<ExecutionNode> optimizedChild =
        optimizeNode(filterNode->releaseChild());

    return tryPushDownFilter(condition, std::move(optimizedChild));
}

std::unique_ptr<ExecutionNode> Optimizer::tryPushDownFilter(
    const Condition& condition,
    std::unique_ptr<ExecutionNode> child
)
{
    if (!child)
    {
        return std::make_unique<FilterNode>(condition, nullptr);
    }

    if (child->getType() != ExecutionNodeType::Join)
    {
        return std::make_unique<FilterNode>(condition, std::move(child));
    }

    JoinNode* joinNode = dynamic_cast<JoinNode*>(child.get());

    std::unordered_set<std::string> referencedTables =
        extractReferencedTables(condition);

    std::unordered_set<std::string> leftTables =
        collectTables(joinNode->getLeftChild());

    std::unordered_set<std::string> rightTables =
        collectTables(joinNode->getRightChild());

    bool belongsToLeft = true;
    for (const std::string& table : referencedTables)
    {
        if (leftTables.find(table) == leftTables.end())
        {
            belongsToLeft = false;
            break;
        }
    }

    bool belongsToRight = true;
    for (const std::string& table : referencedTables)
    {
        if (rightTables.find(table) == rightTables.end())
        {
            belongsToRight = false;
            break;
        }
    }

    if (belongsToLeft)
    {
        std::unique_ptr<ExecutionNode> leftChild = joinNode->releaseLeftChild();
        std::unique_ptr<ExecutionNode> rightChild = joinNode->releaseRightChild();

        leftChild = std::make_unique<FilterNode>(condition, std::move(leftChild));
        leftChild = optimizeNode(std::move(leftChild));

        return std::make_unique<JoinNode>(
            joinNode->getJoinCondition(),
            std::move(leftChild),
            std::move(rightChild)
        );
    }

    if (belongsToRight)
    {
        std::unique_ptr<ExecutionNode> leftChild = joinNode->releaseLeftChild();
        std::unique_ptr<ExecutionNode> rightChild = joinNode->releaseRightChild();

        rightChild = std::make_unique<FilterNode>(condition, std::move(rightChild));
        rightChild = optimizeNode(std::move(rightChild));

        return std::make_unique<JoinNode>(
            joinNode->getJoinCondition(),
            std::move(leftChild),
            std::move(rightChild)
        );
    }

    return std::make_unique<FilterNode>(condition, std::move(child));
}

std::unique_ptr<ExecutionNode> Optimizer::tryPushDownProjection(
    const std::vector<std::string>& requiredColumns,
    std::unique_ptr<ExecutionNode> child
)
{
    if (!child)
    {
        return nullptr;
    }

    ExecutionNodeType nodeType = child->getType();

    if (nodeType == ExecutionNodeType::TableScan)
    {
        return std::make_unique<ProjectionNode>(
            requiredColumns,
            std::move(child)
        );
    }

    if (nodeType == ExecutionNodeType::Projection)
    {
        ProjectionNode* projectionNode = dynamic_cast<ProjectionNode*>(child.get());

        std::vector<std::string> intersection =
            StringUtils::vecIntersection(
                requiredColumns,
                projectionNode->getSelectedFields()
            );

        std::unique_ptr<ExecutionNode> grandChild =
            projectionNode->releaseChild();

        return tryPushDownProjection(intersection, std::move(grandChild));
    }

    if (nodeType == ExecutionNodeType::Join)
    {
        JoinNode* joinNode = dynamic_cast<JoinNode*>(child.get());

        std::vector<std::string> joinColumns =
            extractReferencedColumns(joinNode->getJoinCondition());

        std::vector<std::string> allRequired =
            StringUtils::vecUnion(requiredColumns, joinColumns);

        std::unordered_set<std::string> leftTables =
            collectTables(joinNode->getLeftChild());

        std::unordered_set<std::string> rightTables =
            collectTables(joinNode->getRightChild());

        std::vector<std::string> leftColumns;
        std::vector<std::string> rightColumns;

        for (const std::string& column : allRequired)
        {
            std::vector<std::string> parts = StringUtils::split(column, '.');

            if (parts.size() != 2)
            {
                continue;
            }

            const std::string& tableName = parts[0];

            if (leftTables.find(tableName) != leftTables.end())
            {
                leftColumns.push_back(column);
            }
            else if (rightTables.find(tableName) != rightTables.end())
            {
                rightColumns.push_back(column);
            }
        }

        std::unique_ptr<ExecutionNode> leftChild =
            tryPushDownProjection(leftColumns, joinNode->releaseLeftChild());

        std::unique_ptr<ExecutionNode> rightChild =
            tryPushDownProjection(rightColumns, joinNode->releaseRightChild());

        return std::make_unique<JoinNode>(
            joinNode->getJoinCondition(),
            std::move(leftChild),
            std::move(rightChild)
        );
    }

    if (nodeType == ExecutionNodeType::Filter)
    {
        FilterNode* filterNode = dynamic_cast<FilterNode*>(child.get());

        std::vector<std::string> filterColumns =
            extractReferencedColumns(filterNode->getCondition());

        std::vector<std::string> allRequired =
            StringUtils::vecUnion(requiredColumns, filterColumns);

        Condition condition = filterNode->getCondition();
        std::unique_ptr<ExecutionNode> filterChild =
            filterNode->releaseChild();

        std::unique_ptr<ExecutionNode> pushedChild =
            tryPushDownProjection(allRequired, std::move(filterChild));

        return std::make_unique<FilterNode>(
            condition,
            std::move(pushedChild)
        );
    }

    return std::make_unique<ProjectionNode>(
        requiredColumns,
        std::move(child)
    );
}

std::unordered_set<std::string> Optimizer::extractReferencedTables(const Condition& condition) const
{
    std::unordered_set<std::string> tables;

    auto processOperand = [&tables](const std::string& operand)
        {
            std::vector<std::string> parts = StringUtils::split(operand, '.');

            if (parts.size() == 2)
            {
                tables.insert(parts[0]);
            }
        };

    processOperand(condition.leftOperand);
    processOperand(condition.rightOperand);

    return tables;
}

std::vector<std::string> Optimizer::extractReferencedColumns(const Condition& condition) const
{
    std::vector<std::string> columns;

    auto processOperand = [&columns](const std::string& operand)
        {
            std::vector<std::string> parts = StringUtils::split(operand, '.');

            if (parts.size() == 2)
            {
                columns.push_back(operand);
            }
        };

    processOperand(condition.leftOperand);
    processOperand(condition.rightOperand);

    return StringUtils::removeDuplicates(columns);
}

std::unordered_set<std::string> Optimizer::collectTables(const ExecutionNode* node) const
{
    std::unordered_set<std::string> tables;

    if (!node)
    {
        return tables;
    }

    ExecutionNodeType nodeType = node->getType();

    if (nodeType == ExecutionNodeType::TableScan)
    {
        const TableScanNode* tableScanNode = dynamic_cast<const TableScanNode*>(node);
        tables.insert(tableScanNode->getTableName());
        return tables;
    }

    if (nodeType == ExecutionNodeType::Filter)
    {
        const FilterNode* filterNode = dynamic_cast<const FilterNode*>(node);
        return collectTables(filterNode->getChild());
    }

    if (nodeType == ExecutionNodeType::Projection)
    {
        const ProjectionNode* projectionNode = dynamic_cast<const ProjectionNode*>(node);
        return collectTables(projectionNode->getChild());
    }

    if (nodeType == ExecutionNodeType::Join)
    {
        const JoinNode* joinNode = dynamic_cast<const JoinNode*>(node);

        std::unordered_set<std::string> leftTables =
            collectTables(joinNode->getLeftChild());

        std::unordered_set<std::string> rightTables =
            collectTables(joinNode->getRightChild());

        tables.insert(leftTables.begin(), leftTables.end());
        tables.insert(rightTables.begin(), rightTables.end());
    }

    return tables;
}

int Optimizer::computeRestrictionScore(const ExecutionNode* node) const
{
    if (!node)
    {
        return 0;
    }

    ExecutionNodeType nodeType = node->getType();

    if (nodeType == ExecutionNodeType::TableScan)
    {
        return 0;
    }

    if (nodeType == ExecutionNodeType::Projection)
    {
        const ProjectionNode* projectionNode =
            dynamic_cast<const ProjectionNode*>(node);

        return computeRestrictionScore(projectionNode->getChild());
    }

    if (nodeType == ExecutionNodeType::Filter)
    {
        const FilterNode* filterNode =
            dynamic_cast<const FilterNode*>(node);

        return getOperatorRestrictionWeight(filterNode->getCondition().op)
            + computeRestrictionScore(filterNode->getChild());
    }

    if (nodeType == ExecutionNodeType::Join)
    {
        const JoinNode* joinNode =
            dynamic_cast<const JoinNode*>(node);

        return computeRestrictionScore(joinNode->getLeftChild()) +
            computeRestrictionScore(joinNode->getRightChild());
    }

    return 0;
}

std::unique_ptr<ExecutionNode> Optimizer::reorderJoinTree(std::unique_ptr<ExecutionNode> node)
{
    if (!node)
    {
        return nullptr;
    }

    if (node->getType() != ExecutionNodeType::Join)
    {
        return node;
    }

    std::vector<std::unique_ptr<ExecutionNode>> operands;
    std::vector<Condition> conditions;

    collectJoinConditions(node.get(), conditions);
    collectJoinOperands(std::move(node), operands);

    std::sort(
        operands.begin(),
        operands.end(),
        [this](const std::unique_ptr<ExecutionNode>& left,
            const std::unique_ptr<ExecutionNode>& right)
        {
            return computeRestrictionScore(left.get()) >
                computeRestrictionScore(right.get());
        }
    );

    return rebuildJoinTree(operands, conditions);
}

void Optimizer::collectJoinOperands(
    std::unique_ptr<ExecutionNode> node,
    std::vector<std::unique_ptr<ExecutionNode>>& operands
)
{
    if (!node)
    {
        return;
    }

    if (node->getType() == ExecutionNodeType::Join)
    {
        JoinNode* joinNode = dynamic_cast<JoinNode*>(node.get());

        std::unique_ptr<ExecutionNode> leftChild = joinNode->releaseLeftChild();
        std::unique_ptr<ExecutionNode> rightChild = joinNode->releaseRightChild();

        collectJoinOperands(std::move(leftChild), operands);
        collectJoinOperands(std::move(rightChild), operands);

        return;
    }

    operands.push_back(std::move(node));
}

void Optimizer::collectJoinConditions(
    const ExecutionNode* node,
    std::vector<Condition>& conditions
) const
{
    if (!node)
    {
        return;
    }

    if (node->getType() != ExecutionNodeType::Join)
    {
        return;
    }

    const JoinNode* joinNode = dynamic_cast<const JoinNode*>(node);

    collectJoinConditions(joinNode->getLeftChild(), conditions);
    collectJoinConditions(joinNode->getRightChild(), conditions);

    conditions.push_back(joinNode->getJoinCondition());
}

int Optimizer::getOperatorRestrictionWeight(const std::string& op) const
{
    if (op == "=")
    {
        return 4;
    }

    if (op == "<" || op == ">")
    {
        return 3;
    }

    if (op == "<=" || op == ">=")
    {
        return 2;
    }

    if (op == "<>")
    {
        return 1;
    }

    return 1;
}

std::unique_ptr<ExecutionNode> Optimizer::rebuildJoinTree(
    std::vector<std::unique_ptr<ExecutionNode>>& operands,
    const std::vector<Condition>& conditions
)
{
    if (operands.empty())
    {
        return nullptr;
    }

    if (operands.size() == 1)
    {
        return std::move(operands[0]);
    }

    std::vector<Condition> remainingConditions = conditions;

    std::unique_ptr<ExecutionNode> currentTree = std::move(operands[0]);
    operands.erase(operands.begin());

    while (!operands.empty())
    {
        bool foundCandidate = false;
        std::size_t bestOperandIndex = 0;
        std::size_t bestConditionIndex = 0;
        int bestScore = -1;

        std::unordered_set<std::string> currentTables =
            collectTables(currentTree.get());

        for (std::size_t operandIndex = 0; operandIndex < operands.size(); ++operandIndex)
        {
            std::unordered_set<std::string> operandTables =
                collectTables(operands[operandIndex].get());

            int operandScore = computeRestrictionScore(operands[operandIndex].get());

            for (std::size_t conditionIndex = 0; conditionIndex < remainingConditions.size(); ++conditionIndex)
            {
                Condition selectedCondition = remainingConditions[conditionIndex];

                std::unordered_set<std::string> referencedTables =
                    extractReferencedTables(selectedCondition);

                if (referencedTables.size() != 2)
                {
                    continue;
                }

                auto it = referencedTables.begin();
                const std::string firstTable = *it;
                ++it;
                const std::string secondTable = *it;

                bool firstInCurrent = currentTables.find(firstTable) != currentTables.end();
                bool secondInCurrent = currentTables.find(secondTable) != currentTables.end();

                bool firstInOperand = operandTables.find(firstTable) != operandTables.end();
                bool secondInOperand = operandTables.find(secondTable) != operandTables.end();

                bool canConnect =
                    (firstInCurrent && secondInOperand) ||
                    (secondInCurrent && firstInOperand);

                if (!canConnect)
                {
                    continue;
                }

                if (!foundCandidate || operandScore > bestScore)
                {
                    foundCandidate = true;
                    bestOperandIndex = operandIndex;
                    bestConditionIndex = conditionIndex;
                    bestScore = operandScore;
                }
            }
        }

        if (!foundCandidate)
        {
            break;
        }

        Condition selectedCondition = remainingConditions[bestConditionIndex];

        std::unique_ptr<ExecutionNode> nextOperand =
            std::move(operands[bestOperandIndex]);

        operands.erase(operands.begin() + static_cast<std::ptrdiff_t>(bestOperandIndex));
        remainingConditions.erase(
            remainingConditions.begin() + static_cast<std::ptrdiff_t>(bestConditionIndex)
        );

        currentTree = std::make_unique<JoinNode>(
            selectedCondition,
            std::move(currentTree),
            std::move(nextOperand)
        );
    }

    return currentTree;
}

