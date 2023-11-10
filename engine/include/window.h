#pragma once

#include <GLFW/glfw3.h>

#include "logger.h"

#include "utils/string.h"
#include "utils/thread.h"

DECLARE_LOG_CATEGORY(GLFWInternal);
DECLARE_LOG_CATEGORY(Window);

namespace Window
{
    class Window
    {
    public:
        Window(unsigned int width, unsigned int height, const char* title);
        Window(Window&& other) = delete;
        Window(const Window& other) = delete;

        ~Window();

        Window& operator=(Window&& other) = delete;
        Window& operator=(const Window& other) = delete;

        void Present();
        void Update();

        bool ShouldClose();

        inline unsigned int GetWidth() const noexcept { return width; }
        inline unsigned int GetHeight() const noexcept { return height; }
        inline float GetAspectRatio() const noexcept { return width * 1.0f / height; }
        inline const ::Utils::String& GetTitle() const noexcept { return title; }

        inline GLFWwindow* GetInternalWindow() const noexcept { return window; }

    private:
        GLFWwindow* window;
        unsigned int width;
        unsigned int height;
        ::Utils::String title;

        mutable ::Utils::Mutex mutex;
    };
};

