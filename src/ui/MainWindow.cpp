#include "MainWindow.h"

#include "../app/QueryProcessorService.h"
#include "imgui.h"

MainWindow::MainWindow(const QueryProcessorService& queryProcessorService)
    : m_QueryProcessorService(queryProcessorService)
{
    m_QueryInputPanel.setSql("SELECT Cliente.Nome FROM Cliente;");
}

void MainWindow::render()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("Otimizador de Consultas Root", nullptr, windowFlags);

    renderQuerySection();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderInfoSection();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderBottomSection();

    ImGui::End();

    ImGui::PopStyleVar(2);
}

void MainWindow::processCurrentQuery()
{
    const std::string sql = m_QueryInputPanel.getSql();
    m_CurrentResult = m_QueryProcessorService.process(sql);
    m_HasResult = true;
    m_CurrentPlanView = PlanViewMode::Optimized;
}

void MainWindow::renderQuerySection()
{
    ImGui::Text("Consulta SQL");
    ImGui::Spacing();

    const bool shouldProcess = m_QueryInputPanel.render();

    if (shouldProcess)
    {
        processCurrentQuery();
    }
}

void MainWindow::renderInfoSection() const
{
    ImGui::BeginChild("InfoSection", ImVec2(0.0f, 170.0f), true);

    if (!m_HasResult)
    {
        ImGui::Text("Status");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Nenhuma consulta processada ainda.");
        ImGui::EndChild();
        return;
    }

    ImGui::Text("Status");
    ImGui::Separator();
    ImGui::Spacing();

    if (!m_CurrentResult.success)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Erro no processamento");
        ImGui::Spacing();
        ImGui::TextWrapped("%s", m_CurrentResult.errorMessage.c_str());
        ImGui::EndChild();
        return;
    }

    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Consulta processada com sucesso");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Algebra Relacional");
    ImGui::Spacing();
    m_RelationalAlgebraPanel.render(m_CurrentResult.relationalAlgebra);

    ImGui::EndChild();
}

void MainWindow::renderBottomSection()
{
    if (!m_HasResult || !m_CurrentResult.success)
    {
        ImGui::Text("Planos indisponiveis.");
        return;
    }

    const float splitterWidth = 8.0f;
    const float minLeftWidth = 220.0f;
    const float maxLeftWidth = 800.0f;

    ImVec2 available = ImGui::GetContentRegionAvail();

    if (available.x <= (minLeftWidth + splitterWidth + 200.0f))
    {
        ImGui::Text("Espaco insuficiente para exibir os paineis.");
        return;
    }

    if (m_ExecutionOrderPanelWidth < minLeftWidth)
    {
        m_ExecutionOrderPanelWidth = minLeftWidth;
    }

    if (m_ExecutionOrderPanelWidth > maxLeftWidth)
    {
        m_ExecutionOrderPanelWidth = maxLeftWidth;
    }

    if (m_ExecutionOrderPanelWidth > available.x - splitterWidth - 200.0f)
    {
        m_ExecutionOrderPanelWidth = available.x - splitterWidth - 200.0f;
    }

    const float leftWidth = m_ExecutionOrderPanelWidth;
    const float rightWidth = available.x - leftWidth - splitterWidth;
    const float panelHeight = available.y;

    ImGui::BeginChild("ExecutionOrderPanelContainer", ImVec2(leftWidth, panelHeight), true);
    ImGui::Text("Ordem de Execucao");
    ImGui::Separator();
    ImGui::Spacing();
    m_ExecutionOrderPanel.render(m_CurrentResult.executionOrder);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::InvisibleButton("ExecutionOrderSplitter", ImVec2(splitterWidth, panelHeight));

    if (ImGui::IsItemActive())
    {
        m_ExecutionOrderPanelWidth += ImGui::GetIO().MouseDelta.x;

        if (m_ExecutionOrderPanelWidth < minLeftWidth)
        {
            m_ExecutionOrderPanelWidth = minLeftWidth;
        }

        if (m_ExecutionOrderPanelWidth > maxLeftWidth)
        {
            m_ExecutionOrderPanelWidth = maxLeftWidth;
        }

        const float dynamicMaxWidth = available.x - splitterWidth - 200.0f;
        if (m_ExecutionOrderPanelWidth > dynamicMaxWidth)
        {
            m_ExecutionOrderPanelWidth = dynamicMaxWidth;
        }
    }

    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();

        drawList->AddRectFilled(min, max, IM_COL32(90, 120, 170, 120), 3.0f);
    }

    ImGui::SameLine();

    ImGui::BeginChild("PlanPanelContainer", ImVec2(rightWidth, panelHeight), true);

    ImGui::Text("Plano de Execucao");
    ImGui::Separator();
    ImGui::Spacing();

    renderPlanTabs();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (m_CurrentPlanView == PlanViewMode::Optimized)
    {
        ImGui::Text("Plano Otimizado");
        ImGui::Separator();
        ImGui::Spacing();
        const float graphHeight = ImGui::GetContentRegionAvail().y;

        m_PlanGraphPanel.render(
            m_CurrentResult.optimizedPlan,
            "OptimizedPlanGraph",
            graphHeight);
    }
    else
    {
        ImGui::Text("Plano Original");
        ImGui::Separator();
        ImGui::Spacing();
        const float graphHeight = ImGui::GetContentRegionAvail().y;

        m_PlanGraphPanel.render(
            m_CurrentResult.originalPlan,
            "OriginalPlanGraph",
            graphHeight);
    }

    ImGui::EndChild();
}

void MainWindow::renderPlanTabs()
{
    const bool optimizedSelected = (m_CurrentPlanView == PlanViewMode::Optimized);
    const bool originalSelected = (m_CurrentPlanView == PlanViewMode::Original);

    if (optimizedSelected)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.48f, 0.80f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.33f, 0.53f, 0.85f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.43f, 0.75f, 1.0f));
    }

    if (ImGui::Button("Plano Otimizado", ImVec2(150.0f, 0.0f)))
    {
        m_CurrentPlanView = PlanViewMode::Optimized;
    }

    if (optimizedSelected)
    {
        ImGui::PopStyleColor(3);
    }

    ImGui::SameLine();

    if (originalSelected)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.48f, 0.80f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.33f, 0.53f, 0.85f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.43f, 0.75f, 1.0f));
    }

    if (ImGui::Button("Plano Original", ImVec2(150.0f, 0.0f)))
    {
        m_CurrentPlanView = PlanViewMode::Original;
    }

    if (originalSelected)
    {
        ImGui::PopStyleColor(3);
    }
}