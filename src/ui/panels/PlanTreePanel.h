#pragma once

#include "../../app/PlanNodeView.h"

class PlanTreePanel
{
public:
    void render(const PlanNodeView& rootNode) const;

private:
    void renderNode(const PlanNodeView& node) const;
};