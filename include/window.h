#pragma once

#include <string>

#include <GLFW/glfw3.h>

#include "logger.h"

DECLARE_LOG_CATEGORY(GLFWInternal);
DECLARE_LOG_CATEGORY(Window);

namespace Window
{
    void Init(unsigned int width, unsigned int height, const std::string& title);
    void Destroy();

    void Present();
    void Update();

    bool ShouldClose();

    GLFWwindow* GetInternalWindow();

    unsigned int GetWidth();
    unsigned int GetHeight();
    float GetAspectRatio();
    const std::string& GetTitle();
};

