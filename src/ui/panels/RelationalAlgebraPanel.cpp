#include "RelationalAlgebraPanel.h"

#include "imgui.h"

void RelationalAlgebraPanel::render(const std::string& relationalAlgebra) const
{
    ImGui::Text("Algebra Relacional");
    ImGui::Spacing();

    if (relationalAlgebra.empty())
    {
        ImGui::TextWrapped("Nenhuma algebra relacional disponivel.");
        return;
    }

    ImGui::TextWrapped("%s", relationalAlgebra.c_str());
}