#include <iostream>
#include <string>
#include <vector>

#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "metadata/MetadataCatalog.h"
#include "semantic/SemanticValidator.h"

#include "execution/ExecutionPlan.h"
#include "execution/ExecutionPlanBuilder.h"
#include "execution/ExecutionNode.h"
#include "execution/TableScanNode.h"
#include "execution/FilterNode.h"
#include "execution/JoinNode.h"
#include "execution/ProjectionNode.h"

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
            "select cliente.nome, pedido.datapedido, produto.nome, categoria.descricao "
            "from cliente "
            "join pedido on cliente.idcliente = pedido.cliente_idcliente "
            "join pedido_has_produto on pedido.idpedido = pedido_has_produto.pedido_idpedido "
            "join produto on pedido_has_produto.produto_idproduto = produto.idproduto "
            "join categoria on produto.categoria_idcategoria = categoria.idcategoria "
            "where cliente.idcliente <> 2 "
            "and pedido.valortotalpedido >= 100 "
            "and pedido_has_produto.quantidade >= 2 "
            "and produto.preco >= 50 "
            "and categoria.descricao <> 45";

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
                    << join.onCondition.leftOperand << " "
                    << join.onCondition.op << " "
                    << join.onCondition.rightOperand << "\n";
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
                std::cout << "- " << condition.leftOperand << " "
                    << condition.op << " "
                    << condition.rightOperand << "\n";
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
    }
    catch (const std::exception& e)
    {
        std::cout << "\nERRO: " << e.what() << "\n";
    }

    return 0;
}