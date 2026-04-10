#include "Lexer.h"
#include <cctype>
#include <stdexcept>

#include "../../utils/StringUtils.h"

Lexer::Lexer(const std::string& source) : m_Source(source)
{
    m_Current = 0;
}

std::vector<Token> Lexer::tokenize()
{
    m_Tokens.clear();
    m_Current = 0;

    while (!isAtEnd())
        scanToken();

    addToken(TokenType::EndOfFile, "");
    return m_Tokens;
}

char Lexer::peek()
{
    if (isAtEnd())
        return '\0';

    return m_Source[m_Current];
}

char Lexer::peekNext()
{
    if (m_Current + 1 < m_Source.size())
        return m_Source[m_Current + 1];

    return '\0';
}

char Lexer::advance()
{
    if (isAtEnd())
        return '\0';

    char current = m_Source[m_Current];
    m_Current++;
    return current;
}

bool Lexer::isAtEnd()
{
    return m_Current >= m_Source.size();
}

bool Lexer::match(char expected)
{
    if (peek() == expected)
    {
        advance();
        return true;
    }

    return false;
}

void Lexer::scanToken()
{
    char c = advance();

    if (std::isspace(static_cast<unsigned char>(c)))
        return;

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
    {
        scanIdentifier(c);
        return;
    }

    if (std::isdigit(static_cast<unsigned char>(c)))
    {
        scanNumber(c);
        return;
    }

    if (c == '<' || c == '>' || c == '=')
    {
        scanOperator(c);
        return;
    }

    if (c == '\'')
    {
        scanString();
        return;
    }

    if (c == ',')
    {
        addToken(TokenType::Comma, ",");
        return;
    }

    if (c == '(')
    {
        addToken(TokenType::LeftParen, "(");
        return;
    }

    if (c == ')')
    {
        addToken(TokenType::RightParen, ")");
        return;
    }

    if (c == ';')
    {
        addToken(TokenType::Semicolon, ";");
        return;
    }

    throw std::runtime_error("Erro lexico: caractere invalido: " + std::string(1, c));
}

void Lexer::scanIdentifier(char firstChar)
{
    std::string lexeme;
    lexeme += firstChar;

    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_' || peek() == '.')
    {
        lexeme += advance();
    }

    std::string lowerLexeme = StringUtils::toLowerCopy(lexeme);

    if (lowerLexeme == "select")
        addToken(TokenType::Select, lexeme);
    else if (lowerLexeme == "from")
        addToken(TokenType::From, lexeme);
    else if (lowerLexeme == "where")
        addToken(TokenType::Where, lexeme);
    else if (lowerLexeme == "join")
        addToken(TokenType::Join, lexeme);
    else if (lowerLexeme == "on")
        addToken(TokenType::On, lexeme);
    else if (lowerLexeme == "and")
        addToken(TokenType::And, lexeme);
    else
        addToken(TokenType::Identifier, lowerLexeme);
}

void Lexer::scanNumber(char firstChar)
{
    std::string lexeme;
    lexeme += firstChar;

    while (std::isdigit(static_cast<unsigned char>(peek())))
    {
        lexeme += advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext())))
    {
        lexeme += advance();

        while (std::isdigit(static_cast<unsigned char>(peek())))
        {
            lexeme += advance();
        }
    }

    addToken(TokenType::Number, lexeme);
}

void Lexer::scanOperator(char firstChar)
{
    if (firstChar == '=')
        addToken(TokenType::Equal, "=");
    else if (firstChar == '>')
    {
        if (peek() == '=')
        {
            advance();
            addToken(TokenType::GreaterEqual, ">=");
        }
        else
            addToken(TokenType::Greater, ">");
    }
    else
    {
        if (peek() == '=')
        {
            advance();
            addToken(TokenType::LessEqual, "<=");
        }
        else if (peek() == '>')
        {
            advance();
            addToken(TokenType::NotEqual, "<>");
        }
        else
            addToken(TokenType::Less, "<");
    }
}

void Lexer::scanString()
{
    std::string lexeme;

    while (peek() != '\'' && peek() != '\0')
    {
        lexeme += advance();
    }

    if (peek() == '\0')
        throw std::runtime_error("Erro lexico: string literal nao terminada.");

    advance();
    addToken(TokenType::StringLiteral, lexeme);
}

void Lexer::addToken(TokenType type, const std::string& lexeme)
{
    m_Tokens.push_back(Token{ type, lexeme });
}