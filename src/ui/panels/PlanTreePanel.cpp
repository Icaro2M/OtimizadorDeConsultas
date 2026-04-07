#include "PlanTreePanel.h"

#include "imgui.h"

void PlanTreePanel::render(const PlanNodeView& rootNode) const
{
    if (rootNode.label.empty())
    {
        ImGui::TextWrapped("Nenhum plano disponivel.");
        return;
    }

    renderNode(rootNode);
}

void PlanTreePanel::renderNode(const PlanNodeView& node) const
{
    if (node.children.empty())
    {
        ImGui::BulletText("%s", node.label.c_str());
        return;
    }

    const bool isOpen = ImGui::TreeNode(node.label.c_str());

    if (isOpen)
    {
        for (const PlanNodeView& child : node.children)
        {
            renderNode(child);
        }

        ImGui::TreePop();
    }
}