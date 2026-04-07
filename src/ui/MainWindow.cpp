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
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(1200.0f, 700.0f), ImGuiCond_Once);

    ImGui::Begin("Otimizador de Consultas", nullptr, ImGuiWindowFlags_NoCollapse);

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

void MainWindow::renderPlansSection() const
{
    if (!m_HasResult || !m_CurrentResult.success)
    {
        ImGui::Text("Planos indisponiveis.");
        return;
    }

    ImGui::Text("Planos");
    ImGui::Spacing();

    const float availableWidth = ImGui::GetContentRegionAvail().x;
    const float childWidth = (availableWidth - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    const float childHeight = 260.0f;

    ImGui::BeginChild("PlanoOriginalChild", ImVec2(childWidth, childHeight), true);
    ImGui::Text("Plano Original");
    ImGui::Separator();
    ImGui::Spacing();
    m_PlanTreePanel.render(m_CurrentResult.originalPlan);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("PlanoOtimizadoChild", ImVec2(childWidth, childHeight), true);
    ImGui::Text("Plano Otimizado");
    ImGui::Separator();
    ImGui::Spacing();
    m_PlanTreePanel.render(m_CurrentResult.optimizedPlan);
    ImGui::EndChild();
}