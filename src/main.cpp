#include "ui/ImGuiLayer.h"
#include "ui/MainWindow.h"

#include "app/QueryProcessorService.h"

#include "core/metadata/MetadataCatalog.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

namespace
{
    void glfwErrorCallback(int error, const char* description)
    {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }
}

int main()
{
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
    {
        std::cerr << "Falha ao inicializar GLFW." << std::endl;
        return -1;
    }

    try
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

        GLFWwindow* window = glfwCreateWindow(1280, 720, "Otimizador de Consultas", nullptr, nullptr);

        if (window == nullptr)
        {
            glfwTerminate();
            throw std::runtime_error("Falha ao criar a janela GLFW.");
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        ImGuiLayer imguiLayer;
        imguiLayer.initialize(window, "#version 330");

        MetadataCatalog metadataCatalog;

        QueryProcessorService queryProcessorService(metadataCatalog);
        MainWindow mainWindow(queryProcessorService);

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            int displayWidth = 0;
            int displayHeight = 0;
            glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
            glViewport(0, 0, displayWidth, displayHeight);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            imguiLayer.beginFrame();
            mainWindow.render();
            imguiLayer.endFrame();

            glfwSwapBuffers(window);
        }

        imguiLayer.shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erro fatal: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }
}