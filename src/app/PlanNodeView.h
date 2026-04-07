#pragma once

#include <string>
#include <vector>

struct PlanNodeView
{
    std::string label;
    std::vector<PlanNodeView> children;
};