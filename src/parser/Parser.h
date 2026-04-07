#pragma once

#include "../lexer/Token.h"
#include "../query/Query.h"
#include <vector>
#include <string>

class Parser
{
private:
    const std::vector<Token>& m_Tokens;
    size_t m_Current;

private:
    void parseSelectList(Query& query);
    void parseFrom(Query& query);
    void parseJoin(Query& query);
    void parseWhere(Query& query);

    Condition parseCondition();
    Operand parseOperand();
    Operand buildOperand(const Token& token);

    Token consume(TokenType type, const std::string& errorMessage);
    bool match(TokenType type);
    bool check(TokenType type) const;

    Token advance();
    const Token& peek() const;
    const Token& previous() const;

    bool isAtEnd() const;

public:
    Parser(const std::vector<Token>& tokens);

    Query parse();
};