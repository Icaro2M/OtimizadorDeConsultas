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

    void renderHeader() const;
    void renderQuerySection();
    void renderInfoSection() const;
    void renderPlansSection();
    void renderPlanTabs();

private:
    const QueryProcessorService& m_QueryProcessorService;

    QueryInputPanel m_QueryInputPanel;

    RelationalAlgebraPanel m_RelationalAlgebraPanel;
    PlanGraphPanel m_PlanGraphPanel;
    ExecutionOrderPanel m_ExecutionOrderPanel;

    QueryProcessingResult m_CurrentResult;

    bool m_HasResult = false;
    PlanViewMode m_CurrentPlanView = PlanViewMode::Optimized;
};