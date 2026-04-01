#pragma once

#include "../lexer/Token.h"
#include "../query/Query.h"
#include "../query/JoinClause.h"

#include <string>
#include <vector>

class Parser
{
private:
    std::vector<Token> m_Tokens;
    size_t m_Current;

private:
    void parseSelectList(Query& query);
    void parseFrom(Query& query);
    void parseJoin(Query& query);
    void parseWhere(Query& query);
    Condition parseCondition();

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