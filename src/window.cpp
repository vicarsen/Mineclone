#include "window.h"

#include "files.h"

DEFINE_LOG_CATEGORY(GLFWInternal, spdlog::level::trace, LOGFILE("Window/GLFWInternal.txt"));
DEFINE_LOG_CATEGORY(Window, spdlog::level::trace, LOGFILE("Window/Window.txt"));

namespace Window
{
    static int g_width, g_height;
    static std::string g_title;
    static GLFWwindow* g_window;

    static void GLFWErrorCallback(int error, const char* description)
    {
        ERROR(GLFWInternal, "(error:{}) {}", error, description);
    }

    void Init(unsigned int width, unsigned int height, const std::string& title)
    {
        TRACE(Window, "[Init] (#width:{}) (#height:{}) (#title:{})", width, height, title);
        g_width = width; g_height = height; g_title = title;

        glfwSetErrorCallback(GLFWErrorCallback);
    
        if(!glfwInit())
        {
            CRITICAL(Window, "[Init] GLFW init failure");
            return;
        }

        g_window = glfwCreateWindow(g_width, g_height, g_title.c_str(), NULL, NULL);
        if(g_window == nullptr)
        {
            CRITICAL(Window, "[Init] window creation failure");
            return;
        }
    }

    void Destroy()
    {
        TRACE(Window, "[Destroy] <>");

        if(g_window != nullptr) glfwDestroyWindow(g_window);
        glfwTerminate();
    }

    void Present()
    {
        glfwSwapBuffers(g_window);
    }

    void Update()
    {
        glfwPollEvents();
    }

    bool ShouldClose()
    {
        return glfwWindowShouldClose(g_window);
    }

    GLFWwindow* GetInternalWindow()
    {
        return g_window;
    }

    unsigned int GetWidth()
    {
        return g_width;
    }

    unsigned int GetHeight()
    {
        return g_height;
    }

    float GetAspectRatio()
    {
        return g_width * 1.0f / g_height;
    }

    const std::string& GetTitle()
    {
        return g_title;
    }
};

