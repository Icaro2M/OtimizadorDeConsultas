#pragma once

#include <string>
#include "Condition.h"

struct JoinClause
{
	std::string tableName;
	Condition onCondition;
};