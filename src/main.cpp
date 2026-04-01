#include <iostream>
#include <string>
#include <vector>

#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "metadata/MetadataCatalog.h"
#include "semantic/SemanticValidator.h"

int main()
{
    try
    {
        std::string sqlQuery =
            "select cliente.nome, pedido.datapedido "
            "from cliente "
            "join pedido on cliente.idcliente = pedido.cliente_idcliente "
            "where pedido.valortotalpedido >= 100 and cliente.idcliente <> 2";

        Lexer lexer(sqlQuery);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        Query parsedQuery = parser.parse();

        MetadataCatalog metadataCatalog;
        SemanticValidator validator(metadataCatalog);
        validator.validate(parsedQuery);

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
        std::cout << "\n";

        std::cout << "JOINS:\n";
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
        std::cout << "\n";

        std::cout << "WHERE CONDITIONS:\n";
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
    }
    catch (const std::exception& e)
    {
        std::cout << "\nERRO: " << e.what() << "\n";
    }

    return 0;
}