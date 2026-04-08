#include "Parser.h"

#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens)
    : m_Tokens(tokens), m_Current(0)
{
}

Query Parser::parse()
{
    Query query;

    consume(TokenType::Select, "esperado SELECT no inicio da consulta");
    parseSelectList(query);
    consume(TokenType::From, "esperado FROM apos a lista de campos");
    parseFrom(query);

    while (match(TokenType::Join))
    {
        parseJoin(query);
    }

    if (match(TokenType::Where))
    {
        parseWhere(query);
    }

    match(TokenType::Semicolon);
    consume(TokenType::EndOfFile, "tokens inesperados no final da consulta");

    return query;
}

void Parser::parseSelectList(Query& query)
{
    Token fieldToken = consume(TokenType::Identifier, "esperado um campo apos SELECT");
    query.addSelectField(fieldToken.lexeme);

    while (match(TokenType::Comma))
    {
        Token nextField = consume(TokenType::Identifier, "esperado um campo apos virgula");
        query.addSelectField(nextField.lexeme);
    }
}

void Parser::parseFrom(Query& query)
{
    Token tableToken = consume(TokenType::Identifier, "esperado nome da tabela apos FROM");
    query.setFromTable(tableToken.lexeme);
}

void Parser::parseJoin(Query& query)
{
    Token tableToken = consume(TokenType::Identifier, "esperado nome da tabela apos JOIN");
    consume(TokenType::On, "esperado ON apos a tabela do JOIN");

    Condition condition = parseCondition();
    JoinClause joinClause{ tableToken.lexeme, condition };

    query.addJoin(joinClause);
}

void Parser::parseWhere(Query& query)
{
    query.addWhereCondition(parseCondition());

    while (match(TokenType::And))
    {
        query.addWhereCondition(parseCondition());
    }
}

Condition Parser::parseCondition()
{
    bool hasLeftParen = match(TokenType::LeftParen);

    Operand leftOperand = parseOperand();

    Token opToken;
    if (match(TokenType::Equal) ||
        match(TokenType::Greater) ||
        match(TokenType::Less) ||
        match(TokenType::GreaterEqual) ||
        match(TokenType::LessEqual) ||
        match(TokenType::NotEqual))
    {
        opToken = previous();
    }
    else
    {
        throw std::runtime_error("esperado operador relacional na condicao");
    }

    Operand rightOperand = parseOperand();

    if (hasLeftParen)
    {
        consume(TokenType::RightParen, "esperado ')' ao final da condicao");
    }

    return Condition{ leftOperand, opToken.lexeme, rightOperand };
}

Operand Parser::parseOperand()
{
    if (check(TokenType::Identifier) || check(TokenType::Number) || check(TokenType::StringLiteral))
    {
        Token token = advance();
        return buildOperand(token);
    }

    throw std::runtime_error("esperado operando na condicao");
}

Operand Parser::buildOperand(const Token& token)
{
    if (token.type == TokenType::Identifier)
        return Operand{ token.lexeme, OperandType::Identifier };

    if (token.type == TokenType::Number)
        return Operand{ token.lexeme, OperandType::Number };

    return Operand{ token.lexeme, OperandType::StringLiteral };
}

Token Parser::consume(TokenType type, const std::string& errorMessage)
{
    if (check(type))
        return advance();

    throw std::runtime_error(errorMessage);
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }

    return false;
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd())
        return type == TokenType::EndOfFile;

    return peek().type == type;
}

Token Parser::advance()
{
    if (!isAtEnd())
        m_Current++;

    return previous();
}

const Token& Parser::peek() const
{
    return m_Tokens[m_Current];
}

const Token& Parser::previous() const
{
    return m_Tokens[m_Current - 1];
}

bool Parser::isAtEnd() const
{
    return peek().type == TokenType::EndOfFile;
}