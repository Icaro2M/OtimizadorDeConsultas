#pragma once

#include "Token.h"
#include <vector>

class Lexer
{
private:

	std::string m_Source;
	std::vector<Token> m_Tokens;
	size_t m_Current;

private:
	void scanToken();
	void scanIdentifier(char firstChar);
	void scanNumber(char firstChar);
	void scanOperator(char firstChar);
	void scanString();

	void addToken(TokenType type, const std::string& lexeme);

	bool isAtEnd();

	char advance();
	char peek();
	char peekNext();

	bool match(char expected);

public:

	Lexer(const std::string& source);

	std::vector<Token> tokenize();

};
