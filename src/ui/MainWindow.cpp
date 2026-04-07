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

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("Otimizador de Consultas Root", nullptr, windowFlags);

    renderHeader();

    ImGui::Separator();
    renderQuerySection();

    ImGui::Separator();
    renderInfoSection();

    ImGui::Separator();
    renderPlansSection();

    ImGui::End();
}

void MainWindow::processCurrentQuery()
{
    const std::string sql = m_QueryInputPanel.getSql();
    m_CurrentResult = m_QueryProcessorService.process(sql);
    m_HasResult = true;

    m_CurrentPlanView = PlanViewMode::Optimized;
}

void MainWindow::renderHeader() const
{
    ImGui::Text("Otimizador de Consultas");
    ImGui::Spacing();
}

void MainWindow::renderQuerySection()
{
    ImGui::Text("Consulta SQL");
    ImGui::Spacing();

    const bool shouldProcess = m_QueryInputPanel.render();

    ImGui::Spacing();

    if (shouldProcess)
    {
        processCurrentQuery();
    }
}

void MainWindow::renderInfoSection() const
{
    if (!m_HasResult)
    {
        ImGui::Text("Status: nenhuma consulta processada ainda.");
        return;
    }

    if (!m_CurrentResult.success)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Status: erro no processamento");
        ImGui::Spacing();
        ImGui::TextWrapped("%s", m_CurrentResult.errorMessage.c_str());
        return;
    }

    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Status: consulta processada com sucesso");

    ImGui::Spacing();
    m_RelationalAlgebraPanel.render(m_CurrentResult.relationalAlgebra);

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Spacing();
    m_ExecutionOrderPanel.render(m_CurrentResult.executionOrder);
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

void MainWindow::renderPlansSection()
{
    if (!m_HasResult || !m_CurrentResult.success)
    {
        ImGui::Text("Planos indisponiveis.");
        return;
    }

    ImGui::Text("Plano de Execucao");
    ImGui::Spacing();

    renderPlanTabs();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (m_CurrentPlanView == PlanViewMode::Optimized)
    {
        ImGui::Text("Plano Otimizado");
        ImGui::Separator();
        m_PlanGraphPanel.render(m_CurrentResult.optimizedPlan, "OptimizedPlanGraph");
        return;
    }

    ImGui::Text("Plano Original");
    ImGui::Separator();
    m_PlanGraphPanel.render(m_CurrentResult.originalPlan, "OriginalPlanGraph");
}