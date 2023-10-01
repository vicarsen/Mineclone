#include "window.h"

#include "files.h"

DEFINE_LOG_CATEGORY(GLFWInternal, FILE_LOGGER(trace, LOGFILE("Window/GLFWInternal.txt")));
DEFINE_LOG_CATEGORY(Window, FILE_LOGGER(trace, LOGFILE("Window/Window.txt")));

namespace Window
{
    static void GLFWErrorCallback(int error, const char* description)
    {
        ERROR(GLFWInternal, "(error:{}) {}", error, description);
    }

    Window::Window(unsigned int _width, unsigned int _height, const char* _title) :
        width(_width), height(_height), title(_title)
    {
        glfwSetErrorCallback(GLFWErrorCallback);

        if(!glfwInit())
        {
            CRITICAL(Window, "Failed to initialize GLFW!");
            return;
        }

        window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if(window == nullptr)
        {
            CRITICAL(Window, "Failed to create window!");
            return;
        }
    }

    Window::~Window()
    {
        if(window != nullptr)
            glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::Present()
    {
        glfwSwapBuffers(window);
    }

    void Window::Update()
    {
        glfwPollEvents();
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(window);
    }
};

