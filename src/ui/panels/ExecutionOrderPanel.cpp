#include "ExecutionOrderPanel.h"

#include "imgui.h"

void ExecutionOrderPanel::render(const std::vector<std::string>& executionOrder) const
{
    ImGui::Text("Ordem de Execucao");
    ImGui::Spacing();

    if (executionOrder.empty())
    {
        ImGui::TextWrapped("Nenhuma ordem de execucao disponivel.");
        return;
    }

    for (std::size_t i = 0; i < executionOrder.size(); i++)
    {
        ImGui::BulletText("%d. %s", static_cast<int>(i + 1), executionOrder[i].c_str());
    }
}