#pragma once

#include "../app/QueryProcessingResult.h"

#include "panels/ExecutionOrderPanel.h"
#include "panels/PlanTreePanel.h"
#include "panels/QueryInputPanel.h"
#include "panels/RelationalAlgebraPanel.h"

class QueryProcessorService;

class MainWindow
{
public:
    MainWindow(const QueryProcessorService& queryProcessorService);

    void render();

private:
    void processCurrentQuery();

    void renderHeader() const;
    void renderQuerySection();
    void renderInfoSection() const;
    void renderPlansSection() const;

private:
    const QueryProcessorService& m_QueryProcessorService;

    QueryInputPanel m_QueryInputPanel;
    RelationalAlgebraPanel m_RelationalAlgebraPanel;
    PlanTreePanel m_PlanTreePanel;
    ExecutionOrderPanel m_ExecutionOrderPanel;

    QueryProcessingResult m_CurrentResult;
    bool m_HasResult = false;
};