#pragma once

#include <string>
#include <vector>

#include "PlanNodeView.h"

struct QueryProcessingResult
{
    std::string sql;
    bool success = false;
    std::string errorMessage;

    std::string relationalAlgebra;

    PlanNodeView originalPlan;
    PlanNodeView optimizedPlan;

    std::vector<std::string> executionOrder;
};