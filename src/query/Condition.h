#pragma once

#include <string>

enum class OperandType
{
    Identifier,
    Number,
    StringLiteral
};

struct Operand
{
    std::string value;
    OperandType type;
};

struct Condition
{
    Operand leftOperand;
    std::string op;
    Operand rightOperand;
};