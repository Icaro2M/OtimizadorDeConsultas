#pragma once

#include "../../app/PlanNodeView.h"

class PlanGraphPanel
{
public:
    void render(const PlanNodeView& rootNode, const char* panelId) const;
};