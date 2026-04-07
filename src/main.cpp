#include <iostream>
#include <string>
#include <vector>

#include "core/lexer/Lexer.h"
#include "core/parser/Parser.h"
#include "core/metadata/MetadataCatalog.h"
#include "core/semantic/SemanticValidator.h"

#include "core/execution/ExecutionPlan.h"
#include "core/execution/ExecutionPlanBuilder.h"
#include "core/execution/ExecutionNode.h"
#include "core/execution/TableScanNode.h"
#include "core/execution/FilterNode.h"
#include "core/execution/JoinNode.h"
#include "core/execution/ProjectionNode.h"

#include "core/optimizer/Optimizer.h"

void printExecutionTree(const ExecutionNode* node, int depth = 0)
{
    if (node == nullptr)
    {
        return;
    }

    std::string indent(depth * 4, ' ');
    std::cout << indent << node->toString() << "\n";

    if (const ProjectionNode* projectionNode = dynamic_cast<const ProjectionNode*>(node))
    {
        printExecutionTree(projectionNode->getChild(), depth + 1);
        return;
    }

    if (const FilterNode* filterNode = dynamic_cast<const FilterNode*>(node))
    {
        printExecutionTree(filterNode->getChild(), depth + 1);
        return;
    }

    if (const JoinNode* joinNode = dynamic_cast<const JoinNode*>(node))
    {
        printExecutionTree(joinNode->getLeftChild(), depth + 1);
        printExecutionTree(joinNode->getRightChild(), depth + 1);
        return;
    }

    if (dynamic_cast<const TableScanNode*>(node))
    {
        return;
    }
}

int main()
{
    try
    {
        std::string sqlQuery =
            "SELECT Produto.Nome "
            "from Pedido "
            "join Pedido_has_Produto on Pedido.idPedido = Pedido_has_Produto.Pedido_idPedido "
            "join Produto on Produto.idProduto = Pedido_has_Produto.Produto_idProduto "
            "where Produto.Preco > 100.0 "
            "and Pedido.ValorTotalPedido > 200.0";

        Lexer lexer(sqlQuery);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        Query parsedQuery = parser.parse();

        MetadataCatalog metadataCatalog;
        SemanticValidator validator(metadataCatalog);
        validator.validate(parsedQuery);

        ExecutionPlanBuilder planBuilder;
        ExecutionPlan executionPlan = planBuilder.build(parsedQuery);

        std::cout << "TOKENS:\n";
        for (const Token& t : tokens)
        {
            std::cout << "<" << static_cast<int>(t.type) << " | " << t.lexeme << ">\n";
        }

        std::cout << "\n====================\n";
        std::cout << "QUERY PARSEADA\n";
        std::cout << "====================\n";

        std::cout << "FROM:\n";
        std::cout << parsedQuery.getFromTable() << "\n\n";

        std::cout << "SELECT FIELDS:\n";
        for (const std::string& field : parsedQuery.getSelectFields())
        {
            std::cout << "- " << field << "\n";
        }

        std::cout << "\nJOINS:\n";
        if (parsedQuery.hasJoins())
        {
            for (const JoinClause& join : parsedQuery.getJoins())
            {
                std::cout << "- " << join.tableName << " ON "
                    << join.onCondition.leftOperand.value << " "
                    << join.onCondition.op << " "
                    << join.onCondition.rightOperand.value << "\n";
            }
        }
        else
        {
            std::cout << "nenhum\n";
        }

        std::cout << "\nWHERE CONDITIONS:\n";
        if (parsedQuery.hasWhereConditions())
        {
            for (const Condition& condition : parsedQuery.getWhereConditions())
            {
                std::cout << "- " << condition.leftOperand.value << " "
                    << condition.op << " "
                    << condition.rightOperand.value << "\n";
            }
        }
        else
        {
            std::cout << "nenhuma\n";
        }

        std::cout << "\nVALIDACAO SEMANTICA: OK\n";

        std::cout << "\n====================\n";
        std::cout << "PLANO DE EXECUCAO\n";
        std::cout << "====================\n";
        printExecutionTree(executionPlan.getRoot());

        Optimizer optimizer;
        ExecutionPlan optimizedPlan = optimizer.optimize(std::move(executionPlan));

        std::cout << "\n====================\n";
        std::cout << "PLANO DE EXECUCAO OTIMIZADO\n";
        std::cout << "====================\n";

        printExecutionTree(optimizedPlan.getRoot());
    }
    catch (const std::exception& e)
    {
        std::cout << "\nERRO: " << e.what() << "\n";
    }

    return 0;
}