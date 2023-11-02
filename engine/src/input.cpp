#include "input.h"

#include "files.h"
#include "window.h"

#include "application.h"

DEFINE_LOG_CATEGORY(Input, FILE_LOGGER(trace, LOGFILE("Input.txt")));

namespace Input
{
    InputAction GLFWActionToInputAction(int action)
    {
        switch(action)
        {
        case GLFW_PRESS:
        case GLFW_REPEAT:
            return InputAction::PRESSED;
        case GLFW_RELEASE:
            return InputAction::RELEASED;
        default:
            return InputAction::NONE;
        }
    }

    void __key_callback_fn(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Application::Get()->DispatchEvent<KeyEvent>(key, GLFWActionToInputAction(action));
    }

    void __mouse_button_callback_fn(GLFWwindow* window, int button, int action, int mods)
    {
        Application::Get()->DispatchEvent<ButtonEvent>(button, GLFWActionToInputAction(action));
    }

    void __cursor_pos_callback_fn(GLFWwindow* window, double xpos, double ypos)
    {
        Application::Get()->DispatchEvent<CursorMovedEvent>(glm::vec2 { xpos, ypos });
    }

    InputHandler::InputHandler(GLFWwindow* _window) :
        window(_window)
    {
        glfwSetKeyCallback(window, __key_callback_fn);
        glfwSetMouseButtonCallback(window, __mouse_button_callback_fn);
        glfwSetCursorPosCallback(window, __cursor_pos_callback_fn);

        cursor_mode = CursorMode::DISABLED;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        capture_cursor_movement = true;

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        cursor_position = { xpos, ypos };

        std::memset(keys_pressed, 0, sizeof(keys_pressed));
        std::memset(buttons_pressed, 0, sizeof(buttons_pressed));
    }

    InputHandler::~InputHandler()
    {
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetKeyCallback(window, nullptr);
    }

    void InputHandler::Reset()
    {
        std::memset(keys_just_pressed, 0, sizeof(keys_just_pressed));
        std::memset(keys_just_released, 0, sizeof(keys_just_released));

        std::memset(buttons_just_pressed, 0, sizeof(buttons_just_pressed));
        std::memset(buttons_just_released, 0, sizeof(buttons_just_released));
    }

    void InputHandler::Update()
    {
        if(capture_cursor_movement)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            glm::vec2 new_position = { xpos, ypos };

            cursor_movement = new_position - cursor_position;
            cursor_position = new_position;
        }
    }

    void InputHandler::Handle(const KeyEvent& key_event)
    {
        switch(key_event.action)
        {
        case InputAction::PRESSED:
            if(!keys_pressed[key_event.key])
            {
                keys_just_pressed[key_event.key] = true;
                keys_pressed[key_event.key] = true;
            }
            break;
        case InputAction::RELEASED:
            if(keys_pressed[key_event.key])
            {
                keys_just_released[key_event.key] = true;
                keys_pressed[key_event.key] = false;
            }
            break;
        default:
            break;
        }
    }

    void InputHandler::Handle(const ButtonEvent& button_event)
    {
        switch(button_event.action)
        {
        case InputAction::PRESSED:
            if(!buttons_pressed[button_event.button])
            {
                buttons_just_pressed[button_event.button] = true;
                buttons_pressed[button_event.button] = true;
            }
            break;
        case InputAction::RELEASED:
            if(buttons_pressed[button_event.button])
            {
                buttons_just_released[button_event.button] = true;
                buttons_pressed[button_event.button] = false;
            }
            break;
        default:
            break;
        }
    }

    void InputHandler::Handle(const CursorMovedEvent& cursor_moved_event)
    {
        //cursor_movement = cursor_moved_event.position - cursor_position;
        //cursor_position = cursor_moved_event.position;
    }
    
    bool InputHandler::IsKeyPressed(unsigned int key)
    {
        return keys_pressed[key];
    }

    bool InputHandler::IsKeyJustPressed(unsigned int key)
    {
        return keys_just_pressed[key];
    }

    bool InputHandler::IsKeyReleased(unsigned int key)
    {
        return !keys_pressed[key];
    }

    bool InputHandler::IsKeyJustReleased(unsigned int key)
    {
        return keys_just_released[key];
    }

    bool InputHandler::IsButtonPressed(unsigned int button)
    {
        return buttons_pressed[button];
    }

    bool InputHandler::IsButtonJustPressed(unsigned int button)
    {
        return buttons_just_pressed[button];
    }

    bool InputHandler::IsButtonReleased(unsigned int button)
    {
        return !buttons_pressed[button];
    }

    bool InputHandler::IsButtonJustReleased(unsigned int button)
    {
        return buttons_just_released[button];
    }

    glm::vec2 InputHandler::GetCursorPosition()
    {
        return cursor_position;
    }

    glm::vec2 InputHandler::GetCursorMovement()
    {
        return cursor_movement;
    }

    void InputHandler::SetCursorMode(CursorMode _cursor_mode)
    {
        cursor_mode = _cursor_mode;
        glfwSetInputMode(window, GLFW_CURSOR, cursor_mode == CursorMode::ENABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
};

