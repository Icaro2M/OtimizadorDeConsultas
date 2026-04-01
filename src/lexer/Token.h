#pragma once

#include "TokenType.h"
#include <string>

struct Token
{
public:

	TokenType type;
	std::string lexeme;

};