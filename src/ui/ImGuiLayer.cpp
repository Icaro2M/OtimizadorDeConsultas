#include "ImGuiLayer.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <stdexcept>

void ImGuiLayer::initialize(GLFWwindow* window, const char* glslVersion)
{
    if (m_Initialized)
    {
        return;
    }

    if (window == nullptr)
    {
        throw std::runtime_error("ImGuiLayer recebeu uma janela nula na inicializacao.");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;
    fontConfig.PixelSnapH = false;

    static const ImWchar glyphRanges[] =
    {
        0x0020, 0x00FF,
        0x0370, 0x03FF,
        0x2200, 0x22FF,
        0
    };

    ImFont* customFont = io.Fonts->AddFontFromFileTTF(
        "../../../assets/fonts/NotoSans-Regular.ttf",
        24.0f,
        &fontConfig,
        glyphRanges
    );

    if (customFont == nullptr)
    {
        io.Fonts->AddFontDefault();
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        throw std::runtime_error("Falha ao inicializar o backend GLFW da ImGui.");
    }

    if (!ImGui_ImplOpenGL3_Init(glslVersion))
    {
        throw std::runtime_error("Falha ao inicializar o backend OpenGL3 da ImGui.");
    }

    m_Initialized = true;
}

void ImGuiLayer::beginFrame()
{
    if (!m_Initialized)
    {
        throw std::runtime_error("ImGuiLayer::beginFrame chamado antes da inicializacao.");
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::endFrame()
{
    if (!m_Initialized)
    {
        throw std::runtime_error("ImGuiLayer::endFrame chamado antes da inicializacao.");
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::shutdown()
{
    if (!m_Initialized)
    {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_Initialized = false;
}