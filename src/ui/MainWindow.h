#pragma once

#include "../app/QueryProcessingResult.h"

#include "panels/ExecutionOrderPanel.h"
#include "panels/PlanGraphPanel.h"
#include "panels/QueryInputPanel.h"
#include "panels/RelationalAlgebraPanel.h"

class QueryProcessorService;

class MainWindow
{
public:
    MainWindow(const QueryProcessorService& queryProcessorService);

    void render();

private:
    enum class PlanViewMode
    {
        Optimized,
        Original
    };

private:
    void processCurrentQuery();

    void renderQuerySection();
    void renderInfoSection() const;
    void renderBottomSection();
    void renderPlanTabs();

private:
    const QueryProcessorService& m_QueryProcessorService;

    QueryInputPanel m_QueryInputPanel;
    RelationalAlgebraPanel m_RelationalAlgebraPanel;
    ExecutionOrderPanel m_ExecutionOrderPanel;
    PlanGraphPanel m_PlanGraphPanel;

    QueryProcessingResult m_CurrentResult;

    bool m_HasResult = false;
    PlanViewMode m_CurrentPlanView = PlanViewMode::Optimized;

    float m_ExecutionOrderPanelWidth = 320.0f;
};