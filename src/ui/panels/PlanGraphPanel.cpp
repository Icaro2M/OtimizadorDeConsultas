#include "PlanGraphPanel.h"

#include "imgui.h"

#include <algorithm>
#include <string>
#include <vector>

namespace
{
    struct VisualNode
    {
        int id = 0;
        std::string title;
        std::string content;
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

    constexpr float NODE_WIDTH = 160.0f;
    constexpr float NODE_HEIGHT = 56.0f;
    constexpr float LEVEL_GAP = 95.0f;
    constexpr float SIBLING_GAP = 36.0f;
    constexpr float HEADER_HEIGHT = 22.0f;
    constexpr float PADDING_X = 24.0f;
    constexpr float PADDING_Y = 24.0f;

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

    int buildTree(const PlanNodeView& inputNode, BuildContext& context, int depth)
    {
        VisualNode node;
        node.id = context.nextId++;
        node.title = extractTitle(inputNode.label);
        node.content = extractContent(inputNode.label);
        node.depth = depth;
        node.size = ImVec2(NODE_WIDTH, NODE_HEIGHT);

        applyStyle(node);

        context.nodes.push_back(node);
        const int currentId = node.id;

        for (const PlanNodeView& child : inputNode.children)
        {
            const int childId = buildTree(child, context, depth + 1);
            VisualNode* current = findNode(context.nodes, currentId);
            if (current != nullptr)
            {
                current->children.push_back(childId);
            }
        }

        return currentId;
    }

    float layoutTree(std::vector<VisualNode>& nodes, int nodeId, float& nextLeafX)
    {
        VisualNode* node = findNode(nodes, nodeId);
        if (node == nullptr)
        {
            return 0.0f;
        }

        node->pos.y = static_cast<float>(node->depth) * LEVEL_GAP;

        if (node->children.empty())
        {
            node->pos.x = nextLeafX;
            nextLeafX += NODE_WIDTH + SIBLING_GAP;
            return node->pos.x + NODE_WIDTH * 0.5f;
        }

        std::vector<float> childCenters;
        for (int childId : node->children)
        {
            childCenters.push_back(layoutTree(nodes, childId, nextLeafX));
        }

        const float minCenter = *std::min_element(childCenters.begin(), childCenters.end());
        const float maxCenter = *std::max_element(childCenters.begin(), childCenters.end());

        const float centerX = (minCenter + maxCenter) * 0.5f;
        node->pos.x = centerX - NODE_WIDTH * 0.5f;

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

        return ImVec2(maxX + PADDING_X, maxY + PADDING_Y);
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

    void drawGrid(ImDrawList* drawList, const ImVec2& min, const ImVec2& max)
    {
        const float step = 32.0f;
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

    void drawLinks(ImDrawList* drawList, const std::vector<VisualNode>& nodes)
    {
        const ImU32 linkColor = IM_COL32(125, 175, 245, 255);

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
                    2.5f);
            }
        }
    }

    void drawNode(ImDrawList* drawList, const VisualNode& node)
    {
        const float rounding = 8.0f;
        const ImVec2 min = node.pos;
        const ImVec2 max(node.pos.x + node.size.x, node.pos.y + node.size.y);
        const ImVec2 headerMax(max.x, min.y + HEADER_HEIGHT);

        drawList->AddRectFilled(min, max, node.bodyColor, rounding);
        drawList->AddRectFilled(min, headerMax, node.headerColor, rounding, ImDrawFlags_RoundCornersTop);
        drawList->AddRect(min, max, node.borderColor, rounding, 0, 1.8f);

        drawList->AddText(
            ImVec2(min.x + 8.0f, min.y + 3.0f),
            IM_COL32(255, 255, 255, 255),
            node.title.c_str());

        const std::string visibleContent = ellipsize(node.content, node.size.x - 16.0f);

        drawList->AddText(
            ImVec2(min.x + 8.0f, min.y + HEADER_HEIGHT + 10.0f),
            IM_COL32(235, 235, 235, 255),
            visibleContent.c_str());
    }

    void drawNodes(ImDrawList* drawList, const std::vector<VisualNode>& nodes)
    {
        for (const VisualNode& node : nodes)
        {
            drawNode(drawList, node);
        }
    }
}

void PlanGraphPanel::render(const PlanNodeView& rootNode, const char* panelId) const
{
    if (rootNode.label.empty())
    {
        ImGui::TextWrapped("Nenhum plano disponivel.");
        return;
    }

    ImGui::BeginChild(panelId, ImVec2(0.0f, 470.0f), true);

    const ImVec2 canvasMin = ImGui::GetCursorScreenPos();
    const ImVec2 available = ImGui::GetContentRegionAvail();

    BuildContext context;
    const int rootId = buildTree(rootNode, context, 0);

    float nextLeafX = 0.0f;
    layoutTree(context.nodes, rootId, nextLeafX);
    offsetNodes(context.nodes, PADDING_X, PADDING_Y);

    ImVec2 contentSize = computeBounds(context.nodes);

    float extraOffsetX = 0.0f;
    if (contentSize.x < available.x)
    {
        extraOffsetX = (available.x - contentSize.x) * 0.5f;
        offsetNodes(context.nodes, extraOffsetX, 0.0f);
        contentSize = computeBounds(context.nodes);
    }

    const ImVec2 finalSize(
        std::max(available.x, contentSize.x),
        std::max(available.y, contentSize.y));

    ImGui::InvisibleButton((std::string("canvas_") + panelId).c_str(), finalSize);

    const ImVec2 canvasMax(canvasMin.x + finalSize.x, canvasMin.y + finalSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->PushClipRect(canvasMin, canvasMax, true);

    drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(20, 24, 34, 255), 8.0f);
    drawGrid(drawList, canvasMin, canvasMax);

    for (VisualNode& node : context.nodes)
    {
        node.pos.x += canvasMin.x;
        node.pos.y += canvasMin.y;
    }

    drawLinks(drawList, context.nodes);
    drawNodes(drawList, context.nodes);

    drawList->PopClipRect();

    ImGui::EndChild();
}