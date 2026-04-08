#pragma once

#include "../../app/PlanNodeView.h"

class PlanGraphPanel
{
public:
    void render(const PlanNodeView& rootNode, const char* panelId, float height);

private:
    float m_Zoom = 1.0f;
};