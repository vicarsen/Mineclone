#include "input.h"

#include <cstring>

#include "files.h"
#include "window.h"

DEFINE_LOG_CATEGORY(Input, spdlog::level::trace, LOGFILE("Input.txt"));

namespace Input
{
    static bool g_keys_pressed[GLFW_KEY_LAST + 1];
    static bool g_keys_just_pressed[GLFW_KEY_LAST + 1];
    static bool g_keys_just_released[GLFW_KEY_LAST + 1];
    
    static bool g_buttons_pressed[GLFW_MOUSE_BUTTON_LAST + 1];
    static bool g_buttons_just_pressed[GLFW_MOUSE_BUTTON_LAST + 1];
    static bool g_buttons_just_released[GLFW_MOUSE_BUTTON_LAST + 1];

    static glm::vec2 g_cursor_position, g_cursor_movement;

    bool IsKeyPressed(unsigned int key)
    {
        return g_keys_pressed[key];
    }

    bool IsKeyJustPressed(unsigned int key)
    {
        return g_keys_just_pressed[key];
    }

    bool IsKeyReleased(unsigned int key)
    {
        return !g_keys_pressed[key];
    }

    bool IsKeyJustReleased(unsigned int key)
    {
        return g_keys_just_released[key];
    }

    bool IsButtonPressed(unsigned int button)
    {
        return g_buttons_pressed[button];
    }

    bool IsButtonJustPressed(unsigned int button)
    {
        return g_buttons_just_pressed[button];
    }

    bool IsButtonReleased(unsigned int button)
    {
        return !g_buttons_pressed[button];
    }

    bool IsButtonJustReleased(unsigned int button)
    {
        return g_buttons_just_released[button];
    }

    glm::vec2 GetCursorPosition()
    {
        return g_cursor_position;
    }

    glm::vec2 GetCursorMovement()
    {
        return g_cursor_movement;
    }

    void __key_callback_fn(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        switch(action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
                if(!g_keys_pressed[key])
                    g_keys_just_pressed[key] = true;
                g_keys_pressed[key] = true;
                break;
            case GLFW_RELEASE:
                if(g_keys_pressed[key])
                    g_keys_just_released[key] = true;
                g_keys_pressed[key] = false;
                break;
        }
    }

    void __button_callback_fn(GLFWwindow* window, int button, int action, int mods)
    {
        switch(action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
                if(!g_buttons_pressed[button])
                    g_buttons_just_pressed[button] = true;
                g_buttons_pressed[button] = true;
                break;
            case GLFW_RELEASE:
                if(g_buttons_pressed[button])
                    g_buttons_just_released[button] = true;
                g_buttons_pressed[button] = false;
                break;
        }
    }

    void Init()
    {
        TRACE(Input, "[Init]");

        GLFWwindow* window = Window::GetInternalWindow();
        glfwSetKeyCallback(window, __key_callback_fn);
        glfwSetMouseButtonCallback(window, __button_callback_fn);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
        TRACE(Input, "[Init] return");
    }

    void Destroy()
    {
        TRACE(Input, "[Destroy]");

        GLFWwindow* window = Window::GetInternalWindow();
        glfwSetKeyCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
    
        TRACE(Input, "[Destroy] return");
    }

    void PreUpdate()
    {
        std::memset(g_keys_just_pressed, 0, sizeof(g_keys_just_pressed));
        std::memset(g_keys_just_released, 0, sizeof(g_keys_just_released));

        std::memset(g_buttons_just_pressed, 0, sizeof(g_buttons_just_pressed));
        std::memset(g_buttons_just_released, 0, sizeof(g_buttons_just_released));
    }

    void Update()
    {
        GLFWwindow* window = Window::GetInternalWindow();

        glm::vec2 old_position = g_cursor_position;

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        g_cursor_position = { x, y };

        g_cursor_movement = g_cursor_position - old_position;
    }
};

