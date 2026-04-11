#include "PlanGraphPanel.h"

#include "imgui.h"

#include <algorithm>
#include <string>
#include <vector>

namespace
{
    struct LayoutConfig
    {
        float nodeWidth = 160.0f;
        float nodeHeight = 56.0f;
        float levelGap = 95.0f;
        float siblingGap = 36.0f;
        float headerHeight = 22.0f;
        float paddingX = 24.0f;
        float paddingY = 24.0f;
    };

    struct VisualNode
    {
        int id = 0;
        std::string title;
        std::string content;
        std::string fullLabel;
        std::vector<int> children;

        int depth = 0;

        ImVec2 pos = ImVec2(0.0f, 0.0f);
        ImVec2 size = ImVec2(160.0f, 56.0f);

        ImU32 headerColor = 0;
        ImU32 bodyColor = 0;
        ImU32 borderColor = 0;
    };

    struct BuildContext
    {
        std::vector<VisualNode> nodes;
        int nextId = 1;
    };

    std::string trim(const std::string& text)
    {
        const std::string whitespace = " \t\n\r";
        const size_t begin = text.find_first_not_of(whitespace);

        if (begin == std::string::npos)
        {
            return "";
        }

        const size_t end = text.find_last_not_of(whitespace);
        return text.substr(begin, end - begin + 1);
    }

    std::string extractTitle(const std::string& label)
    {
        const size_t openParen = label.find('(');

        if (openParen == std::string::npos)
        {
            return trim(label);
        }

        return trim(label.substr(0, openParen));
    }

    std::string extractContent(const std::string& label)
    {
        const size_t openParen = label.find('(');
        const size_t closeParen = label.rfind(')');

        if (openParen == std::string::npos || closeParen == std::string::npos || closeParen <= openParen)
        {
            return "";
        }

        return trim(label.substr(openParen + 1, closeParen - openParen - 1));
    }

    void applyStyle(VisualNode& node)
    {
        if (node.title == "Projection")
        {
            node.headerColor = IM_COL32(80, 175, 95, 255);
            node.bodyColor = IM_COL32(44, 74, 49, 255);
            node.borderColor = IM_COL32(110, 215, 130, 255);
            return;
        }

        if (node.title == "Filter")
        {
            node.headerColor = IM_COL32(195, 160, 55, 255);
            node.bodyColor = IM_COL32(88, 74, 35, 255);
            node.borderColor = IM_COL32(230, 195, 85, 255);
            return;
        }

        if (node.title == "Join")
        {
            node.headerColor = IM_COL32(70, 120, 205, 255);
            node.bodyColor = IM_COL32(40, 55, 90, 255);
            node.borderColor = IM_COL32(100, 155, 245, 255);
            return;
        }

        if (node.title == "TableScan")
        {
            node.headerColor = IM_COL32(130, 130, 140, 255);
            node.bodyColor = IM_COL32(58, 60, 72, 255);
            node.borderColor = IM_COL32(180, 180, 190, 255);
            return;
        }

        node.headerColor = IM_COL32(120, 100, 160, 255);
        node.bodyColor = IM_COL32(62, 50, 80, 255);
        node.borderColor = IM_COL32(160, 130, 210, 255);
    }

    VisualNode* findNode(std::vector<VisualNode>& nodes, int id)
    {
        for (VisualNode& node : nodes)
        {
            if (node.id == id)
            {
                return &node;
            }
        }

        return nullptr;
    }

    const VisualNode* findNode(const std::vector<VisualNode>& nodes, int id)
    {
        for (const VisualNode& node : nodes)
        {
            if (node.id == id)
            {
                return &node;
            }
        }

        return nullptr;
    }

    int buildTree(const PlanNodeView& inputNode, BuildContext& context, int depth, const LayoutConfig& layoutConfig)
    {
        VisualNode node;
        node.id = context.nextId++;
        node.title = extractTitle(inputNode.label);
        node.content = extractContent(inputNode.label);
        node.fullLabel = inputNode.label;
        node.depth = depth;
        node.size = ImVec2(layoutConfig.nodeWidth, layoutConfig.nodeHeight);

        applyStyle(node);

        context.nodes.push_back(node);
        const int currentId = node.id;

        for (const PlanNodeView& child : inputNode.children)
        {
            const int childId = buildTree(child, context, depth + 1, layoutConfig);

            VisualNode* current = findNode(context.nodes, currentId);
            if (current != nullptr)
            {
                current->children.push_back(childId);
            }
        }

        return currentId;
    }

    float layoutTree(std::vector<VisualNode>& nodes, int nodeId, float& nextLeafX, const LayoutConfig& layoutConfig)
    {
        VisualNode* node = findNode(nodes, nodeId);
        if (node == nullptr)
        {
            return 0.0f;
        }

        node->pos.y = static_cast<float>(node->depth) * layoutConfig.levelGap;

        if (node->children.empty())
        {
            node->pos.x = nextLeafX;
            nextLeafX += layoutConfig.nodeWidth + layoutConfig.siblingGap;
            return node->pos.x + layoutConfig.nodeWidth * 0.5f;
        }

        std::vector<float> childCenters;
        childCenters.reserve(node->children.size());

        for (int childId : node->children)
        {
            childCenters.push_back(layoutTree(nodes, childId, nextLeafX, layoutConfig));
        }

        const float minCenter = *std::min_element(childCenters.begin(), childCenters.end());
        const float maxCenter = *std::max_element(childCenters.begin(), childCenters.end());

        const float centerX = (minCenter + maxCenter) * 0.5f;
        node->pos.x = centerX - layoutConfig.nodeWidth * 0.5f;

        return centerX;
    }

    void offsetNodes(std::vector<VisualNode>& nodes, float dx, float dy)
    {
        for (VisualNode& node : nodes)
        {
            node.pos.x += dx;
            node.pos.y += dy;
        }
    }

    ImVec2 computeBounds(const std::vector<VisualNode>& nodes)
    {
        float maxX = 0.0f;
        float maxY = 0.0f;

        for (const VisualNode& node : nodes)
        {
            maxX = std::max(maxX, node.pos.x + node.size.x);
            maxY = std::max(maxY, node.pos.y + node.size.y);
        }

        return ImVec2(maxX, maxY);
    }

    std::string ellipsize(const std::string& text, float maxWidth)
    {
        if (text.empty())
        {
            return "";
        }

        if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
        {
            return text;
        }

        std::string result;
        for (char ch : text)
        {
            std::string candidate = result + ch + "...";
            if (ImGui::CalcTextSize(candidate.c_str()).x > maxWidth)
            {
                break;
            }
            result.push_back(ch);
        }

        return result + "...";
    }

    void drawGrid(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float zoom)
    {
        const float step = 32.0f * zoom;
        if (step < 12.0f)
        {
            return;
        }

        const ImU32 color = IM_COL32(90, 100, 120, 30);

        for (float x = min.x; x < max.x; x += step)
        {
            drawList->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), color);
        }

        for (float y = min.y; y < max.y; y += step)
        {
            drawList->AddLine(ImVec2(min.x, y), ImVec2(max.x, y), color);
        }
    }

    void drawLinks(ImDrawList* drawList, const std::vector<VisualNode>& nodes, float zoom)
    {
        const ImU32 linkColor = IM_COL32(125, 175, 245, 255);
        const float thickness = std::max(1.5f, 2.5f * zoom);

        for (const VisualNode& parent : nodes)
        {
            for (int childId : parent.children)
            {
                const VisualNode* child = findNode(nodes, childId);
                if (child == nullptr)
                {
                    continue;
                }

                const ImVec2 start(
                    parent.pos.x + parent.size.x * 0.5f,
                    parent.pos.y + parent.size.y);

                const ImVec2 end(
                    child->pos.x + child->size.x * 0.5f,
                    child->pos.y);

                const float midY = (start.y + end.y) * 0.5f;

                drawList->AddBezierCubic(
                    start,
                    ImVec2(start.x, midY),
                    ImVec2(end.x, midY),
                    end,
                    linkColor,
                    thickness);
            }
        }
    }

    void drawNode(ImDrawList* drawList, const VisualNode& node, const LayoutConfig& layoutConfig, float zoom)
    {
        const float rounding = 8.0f * zoom;
        const ImVec2 min = node.pos;
        const ImVec2 max(node.pos.x + node.size.x, node.pos.y + node.size.y);
        const ImVec2 headerMax(max.x, min.y + layoutConfig.headerHeight);

        drawList->AddRectFilled(min, max, node.bodyColor, rounding);
        drawList->AddRectFilled(min, headerMax, node.headerColor, rounding, ImDrawFlags_RoundCornersTop);
        drawList->AddRect(min, max, node.borderColor, rounding, 0, std::max(1.2f, 1.8f * zoom));

        drawList->AddText(
            ImGui::GetFont(),
            ImGui::GetFontSize() * zoom,
            ImVec2(min.x + 8.0f * zoom, min.y + 3.0f * zoom),
            IM_COL32(255, 255, 255, 255),
            node.title.c_str());

        const std::string visibleContent = ellipsize(node.content, node.size.x - 16.0f * zoom);

        drawList->AddText(
            ImGui::GetFont(),
            ImGui::GetFontSize() * zoom,
            ImVec2(min.x + 8.0f * zoom, min.y + layoutConfig.headerHeight + 8.0f * zoom),
            IM_COL32(235, 235, 235, 255),
            visibleContent.c_str());
    }

    bool isMouseHoveringNode(const VisualNode& node)
    {
        const ImVec2 mousePos = ImGui::GetIO().MousePos;

        const bool insideX = mousePos.x >= node.pos.x && mousePos.x <= (node.pos.x + node.size.x);
        const bool insideY = mousePos.y >= node.pos.y && mousePos.y <= (node.pos.y + node.size.y);

        return insideX && insideY;
    }

    void drawNodeTooltip(const VisualNode& node)
    {
        if (!isMouseHoveringNode(node))
        {
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(30, 30, 35, 245)); 
        ImGui::PushStyleColor(ImGuiCol_Border, node.borderColor); 

        if (ImGui::BeginTooltip())
        {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", node.title.c_str());

            ImGui::Separator();
            ImGui::Spacing();

            float maxWidth = ImGui::GetFontSize() * 35.0f;

            ImGui::PushTextWrapPos(maxWidth);
            ImGui::TextUnformatted(node.fullLabel.c_str());
            ImGui::PopTextWrapPos();

            ImGui::Spacing();
            ImGui::EndTooltip();
        }

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }

    void drawNodes(ImDrawList* drawList, const std::vector<VisualNode>& nodes, const LayoutConfig& layoutConfig, float zoom)
    {
        for (const VisualNode& node : nodes)
        {
            drawNode(drawList, node, layoutConfig, zoom);
        }

        for (const VisualNode& node : nodes)
        {
            drawNodeTooltip(node);
        }
    }

    LayoutConfig buildResponsiveLayout(const ImVec2& available, float requestedHeight)
    {
        LayoutConfig config;

        const float effectiveWidth = std::max(available.x, 300.0f);
        const float effectiveHeight = std::max(requestedHeight, 220.0f);

        config.nodeWidth = std::clamp(effectiveWidth * 0.14f, 135.0f, 180.0f);
        config.nodeHeight = std::clamp(effectiveHeight * 0.11f, 48.0f, 64.0f);
        config.levelGap = std::clamp(effectiveHeight * 0.17f, 72.0f, 110.0f);
        config.siblingGap = std::clamp(effectiveWidth * 0.03f, 20.0f, 42.0f);
        config.headerHeight = std::clamp(config.nodeHeight * 0.38f, 20.0f, 26.0f);
        config.paddingX = std::clamp(effectiveWidth * 0.02f, 18.0f, 28.0f);
        config.paddingY = std::clamp(effectiveHeight * 0.04f, 18.0f, 28.0f);

        return config;
    }

    void applyZoomToNodes(std::vector<VisualNode>& nodes, LayoutConfig& layoutConfig, float zoom)
    {
        layoutConfig.nodeWidth *= zoom;
        layoutConfig.nodeHeight *= zoom;
        layoutConfig.levelGap *= zoom;
        layoutConfig.siblingGap *= zoom;
        layoutConfig.headerHeight *= zoom;
        layoutConfig.paddingX *= zoom;
        layoutConfig.paddingY *= zoom;

        for (VisualNode& node : nodes)
        {
            node.size.x *= zoom;
            node.size.y *= zoom;
            node.pos.x *= zoom;
            node.pos.y *= zoom;
        }
    }
}

void PlanGraphPanel::render(const PlanNodeView& rootNode, const char* panelId, float height)
{
    if (rootNode.label.empty())
    {
        ImGui::TextWrapped("Nenhum plano disponivel.");
        return;
    }

    ImGui::PushID(panelId);

    if (ImGui::Button("-"))
    {
        m_Zoom = std::max(0.60f, m_Zoom - 0.10f);
    }

    ImGui::SameLine();

    if (ImGui::Button("100%"))
    {
        m_Zoom = 1.0f;
    }

    ImGui::SameLine();

    if (ImGui::Button("+"))
    {
        m_Zoom = std::min(1.80f, m_Zoom + 0.10f);
    }

    ImGui::SameLine();
    ImGui::Text("Zoom: %.0f%%", m_Zoom * 100.0f);

    ImGui::BeginChild("GraphPanelChild", ImVec2(0.0f, height), true, ImGuiWindowFlags_HorizontalScrollbar);

    const bool panelHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
    const ImGuiIO& io = ImGui::GetIO();

    if (panelHovered && io.KeyCtrl && io.MouseWheel != 0.0f)
    {
        const float step = 0.10f;
        if (io.MouseWheel > 0.0f)
        {
            m_Zoom = std::min(1.80f, m_Zoom + step);
        }
        else if (io.MouseWheel < 0.0f)
        {
            m_Zoom = std::max(0.60f, m_Zoom - step);
        }
    }

    const ImVec2 canvasMin = ImGui::GetCursorScreenPos();
    const ImVec2 available = ImGui::GetContentRegionAvail();

    LayoutConfig layoutConfig = buildResponsiveLayout(available, height);

    BuildContext context;
    const int rootId = buildTree(rootNode, context, 0, layoutConfig);

    float nextLeafX = 0.0f;
    layoutTree(context.nodes, rootId, nextLeafX, layoutConfig);

    offsetNodes(context.nodes, layoutConfig.paddingX, layoutConfig.paddingY);

    applyZoomToNodes(context.nodes, layoutConfig, m_Zoom);

    ImVec2 contentSize = computeBounds(context.nodes);

    if (contentSize.x < available.x)
    {
        const float extraOffsetX = (available.x - contentSize.x) * 0.5f;
        offsetNodes(context.nodes, extraOffsetX, 0.0f);
        contentSize = computeBounds(context.nodes);
    }

    if (contentSize.y < available.y)
    {
        const float extraOffsetY = (available.y - contentSize.y) * 0.18f;
        offsetNodes(context.nodes, 0.0f, extraOffsetY);
        contentSize = computeBounds(context.nodes);
    }

    const ImVec2 finalSize(
        std::max(available.x, contentSize.x + layoutConfig.paddingX),
        std::max(available.y, contentSize.y + layoutConfig.paddingY));

    ImGui::InvisibleButton("canvas", finalSize);

    const ImVec2 canvasMax(canvasMin.x + finalSize.x, canvasMin.y + finalSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect(canvasMin, canvasMax, true);

    drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(20, 24, 34, 255), 8.0f);
    drawGrid(drawList, canvasMin, canvasMax, m_Zoom);

    for (VisualNode& node : context.nodes)
    {
        node.pos.x += canvasMin.x;
        node.pos.y += canvasMin.y;
    }

    drawLinks(drawList, context.nodes, m_Zoom);
    drawNodes(drawList, context.nodes, layoutConfig, m_Zoom);

    drawList->PopClipRect();

    ImGui::EndChild();
    ImGui::PopID();
}