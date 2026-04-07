#include "QueryInputPanel.h"

#include "imgui.h"

#include <algorithm>
#include <cstring>

QueryInputPanel::QueryInputPanel()
{
    m_Buffer.fill('\0');
}

bool QueryInputPanel::render()
{
    bool shouldProcess = false;

    ImGui::InputTextMultiline(
        "##sql_input",
        m_Buffer.data(),
        m_Buffer.size(),
        ImVec2(-1.0f, 120.0f)
    );

    ImGui::Spacing();

    if (ImGui::Button("Processar", ImVec2(120.0f, 0.0f)))
    {
        shouldProcess = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Limpar", ImVec2(120.0f, 0.0f)))
    {
        m_Buffer.fill('\0');
    }

    return shouldProcess;
}

std::string QueryInputPanel::getSql() const
{
    return std::string(m_Buffer.data());
}

void QueryInputPanel::setSql(const std::string& sql)
{
    m_Buffer.fill('\0');

    const std::size_t copySize = std::min(sql.size(), m_Buffer.size() - 1);
    std::memcpy(m_Buffer.data(), sql.c_str(), copySize);
    m_Buffer[copySize] = '\0';
}