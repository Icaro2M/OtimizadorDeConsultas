#include "QueryProcessorService.h"

#include "../core/lexer/Lexer.h"
#include "../core/parser/Parser.h"
#include "../core/semantic/SemanticValidator.h"
#include "../core/query/Query.h"
#include "../core/execution/ExecutionPlanBuilder.h"
#include "../core/optimizer/Optimizer.h"

#include "../core/execution/TableScanNode.h"
#include "../core/execution/JoinNode.h"
#include "../core/execution/ProjectionNode.h"
#include "../core/execution/FilterNode.h"

#include "../utils/StringUtils.h"

#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

QueryProcessorService::QueryProcessorService(const MetadataCatalog& metadataCatalog)
    : m_MetadataCatalog(metadataCatalog)
{
}

QueryProcessingResult QueryProcessorService::process(const std::string& sql) const
{
    QueryProcessingResult result;
    result.sql = sql;

    try
    {
        Lexer lexer(sql);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        Query parsedQuery = parser.parse();

        SemanticValidator validator(m_MetadataCatalog);
        validator.validate(parsedQuery);

        ExecutionPlanBuilder planBuilder;
        ExecutionPlan originalPlan = planBuilder.build(parsedQuery);

        if (originalPlan.getRoot() == nullptr)
        {
            result.errorMessage = "Plano de execução original inválido: raiz nula.";
            return result;
        }

        result.originalPlan = buildPlanNodeView(*originalPlan.getRoot());
        result.relationalAlgebra = buildRelationalAlgebra(originalPlan);

        Optimizer optimizer;
        ExecutionPlan optimizedPlan = optimizer.optimize(std::move(originalPlan));

        if (optimizedPlan.getRoot() == nullptr)
        {
            result.errorMessage = "Plano de execução otimizado inválido: raiz nula.";
            return result;
        }

        result.optimizedPlan = buildPlanNodeView(*optimizedPlan.getRoot());
        fillExecutionOrder(*optimizedPlan.getRoot(), result.executionOrder);

        result.success = true;
        return result;
    }
    catch (const std::exception& e)
    {
        result.errorMessage = e.what();
        return result;
    }
}

PlanNodeView QueryProcessorService::buildPlanNodeView(const ExecutionNode& node) const
{
    PlanNodeView nodeView;

    if (node.getType() == ExecutionNodeType::TableScan)
    {
        const TableScanNode* tableScanNode = dynamic_cast<const TableScanNode*>(&node);

        if (tableScanNode != nullptr)
        {
            nodeView.label = "TableScan(" + tableScanNode->getTableName() + ")";
        }

        return nodeView;
    }

    if (node.getType() == ExecutionNodeType::Join)
    {
        const JoinNode* joinNode = dynamic_cast<const JoinNode*>(&node);

        if (joinNode != nullptr)
        {
            const Condition condition = joinNode->getJoinCondition();

            nodeView.label =
                "Join(" +
                condition.leftOperand.value + " " +
                condition.op + " " +
                condition.rightOperand.value + ")";

            if (joinNode->getLeftChild() != nullptr)
            {
                nodeView.children.push_back(buildPlanNodeView(*joinNode->getLeftChild()));
            }

            if (joinNode->getRightChild() != nullptr)
            {
                nodeView.children.push_back(buildPlanNodeView(*joinNode->getRightChild()));
            }
        }

        return nodeView;
    }

    if (node.getType() == ExecutionNodeType::Filter)
    {
        const FilterNode* filterNode = dynamic_cast<const FilterNode*>(&node);

        if (filterNode != nullptr)
        {
            const Condition condition = filterNode->getCondition();

            nodeView.label =
                "Filter(" +
                condition.leftOperand.value + " " +
                condition.op + " " +
                condition.rightOperand.value + ")";

            if (filterNode->getChild() != nullptr)
            {
                nodeView.children.push_back(buildPlanNodeView(*filterNode->getChild()));
            }
        }

        return nodeView;
    }

    if (node.getType() == ExecutionNodeType::Projection)
    {
        const ProjectionNode* projectionNode = dynamic_cast<const ProjectionNode*>(&node);

        if (projectionNode != nullptr)
        {
            const std::vector<std::string> fields = projectionNode->getSelectedFields();
            const std::string strFields = StringUtils::vecToString(fields);

            nodeView.label = "Projection(" + strFields + ")";

            if (projectionNode->getChild() != nullptr)
            {
                nodeView.children.push_back(buildPlanNodeView(*projectionNode->getChild()));
            }
        }

        return nodeView;
    }

    nodeView.label = "UnknownNode";
    return nodeView;
}

void QueryProcessorService::fillExecutionOrder(const ExecutionNode& node, std::vector<std::string>& executionOrder) const
{
    if (node.getType() == ExecutionNodeType::TableScan)
    {
        const TableScanNode* tableScanNode = dynamic_cast<const TableScanNode*>(&node);

        if (tableScanNode != nullptr)
        {
            executionOrder.push_back("TableScan(" + tableScanNode->getTableName() + ")");
        }

        return;
    }

    if (node.getType() == ExecutionNodeType::Join)
    {
        const JoinNode* joinNode = dynamic_cast<const JoinNode*>(&node);

        if (joinNode != nullptr)
        {
            if (joinNode->getLeftChild() != nullptr)
            {
                fillExecutionOrder(*joinNode->getLeftChild(), executionOrder);
            }

            if (joinNode->getRightChild() != nullptr)
            {
                fillExecutionOrder(*joinNode->getRightChild(), executionOrder);
            }

            const Condition condition = joinNode->getJoinCondition();

            executionOrder.push_back(
                "Join(" +
                condition.leftOperand.value + " " +
                condition.op + " " +
                condition.rightOperand.value + ")"
            );
        }

        return;
    }

    if (node.getType() == ExecutionNodeType::Filter)
    {
        const FilterNode* filterNode = dynamic_cast<const FilterNode*>(&node);

        if (filterNode != nullptr)
        {
            if (filterNode->getChild() != nullptr)
            {
                fillExecutionOrder(*filterNode->getChild(), executionOrder);
            }

            const Condition condition = filterNode->getCondition();

            executionOrder.push_back(
                "Filter(" +
                condition.leftOperand.value + " " +
                condition.op + " " +
                condition.rightOperand.value + ")"
            );
        }

        return;
    }

    if (node.getType() == ExecutionNodeType::Projection)
    {
        const ProjectionNode* projectionNode = dynamic_cast<const ProjectionNode*>(&node);

        if (projectionNode != nullptr)
        {
            if (projectionNode->getChild() != nullptr)
            {
                fillExecutionOrder(*projectionNode->getChild(), executionOrder);
            }

            const std::vector<std::string> fields = projectionNode->getSelectedFields();
            const std::string strFields = StringUtils::vecToString(fields);

            executionOrder.push_back("Projection(" + strFields + ")");
        }

        return;
    }
}

std::string QueryProcessorService::buildRelationalAlgebra(const ExecutionPlan& plan) const
{
    if (plan.getRoot() == nullptr)
    {
        return "";
    }

    std::function<std::string(const ExecutionNode&)> buildExpression =
        [&](const ExecutionNode& node) -> std::string
        {
            if (node.getType() == ExecutionNodeType::TableScan)
            {
                const TableScanNode* tableScanNode = dynamic_cast<const TableScanNode*>(&node);

                if (tableScanNode != nullptr)
                {
                    return tableScanNode->getTableName();
                }

                return "UnknownTable";
            }

            if (node.getType() == ExecutionNodeType::Filter)
            {
                const FilterNode* filterNode = dynamic_cast<const FilterNode*>(&node);

                if (filterNode != nullptr)
                {
                    const Condition condition = filterNode->getCondition();

                    const std::string predicate =
                        condition.leftOperand.value + " " +
                        condition.op + " " +
                        condition.rightOperand.value;

                    if (filterNode->getChild() != nullptr)
                    {
                        return "σ[" + predicate + "](" + buildExpression(*filterNode->getChild()) + ")";
                    }

                    return "σ[" + predicate + "](?)";
                }

                return "σ[?](?)";
            }

            if (node.getType() == ExecutionNodeType::Projection)
            {
                const ProjectionNode* projectionNode = dynamic_cast<const ProjectionNode*>(&node);

                if (projectionNode != nullptr)
                {
                    const std::vector<std::string> fields = projectionNode->getSelectedFields();
                    const std::string strFields = StringUtils::vecToString(fields);

                    if (projectionNode->getChild() != nullptr)
                    {
                        return "π[" + strFields + "](" + buildExpression(*projectionNode->getChild()) + ")";
                    }

                    return "π[" + strFields + "](?)";
                }

                return "π[?](?)";
            }

            if (node.getType() == ExecutionNodeType::Join)
            {
                const JoinNode* joinNode = dynamic_cast<const JoinNode*>(&node);

                if (joinNode != nullptr)
                {
                    const Condition condition = joinNode->getJoinCondition();

                    const std::string predicate =
                        condition.leftOperand.value + " " +
                        condition.op + " " +
                        condition.rightOperand.value;

                    const std::string leftExpr =
                        (joinNode->getLeftChild() != nullptr)
                        ? buildExpression(*joinNode->getLeftChild())
                        : "?";

                    const std::string rightExpr =
                        (joinNode->getRightChild() != nullptr)
                        ? buildExpression(*joinNode->getRightChild())
                        : "?";

                    return "(" + leftExpr + ") ⋈[" + predicate + "] (" + rightExpr + ")";
                }

                return "(?) ⋈[?] (?)";
            }

            return "UnknownNode";
        };

    return buildExpression(*plan.getRoot());
}