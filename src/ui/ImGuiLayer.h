#pragma once

struct GLFWwindow;

class ImGuiLayer
{
public:
    void initialize(GLFWwindow* window, const char* glslVersion);
    void beginFrame();
    void endFrame();
    void shutdown();

private:
    bool m_Initialized = false;
};