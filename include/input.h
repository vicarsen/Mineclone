#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "logger.h"
#include "events.h"

DECLARE_LOG_CATEGORY(Input);

namespace Input
{
    enum class InputAction { NONE = 0, PRESSED, RELEASED };

    struct KeyEvent
    {
        unsigned int key;
        InputAction action;
    };

    struct ButtonEvent
    {
        unsigned int button;
        InputAction action;
    };

    struct CursorMovedEvent
    {
        glm::vec2 position;
    };

    InputAction GLFWActionToInputAction(int action);

    class InputHandler : public Events::EventHandler<KeyEvent>, public Events::EventHandler<ButtonEvent>, public Events::EventHandler<CursorMovedEvent>
    {
    public:
        InputHandler(GLFWwindow* window);
        InputHandler(InputHandler&& other) = delete;
        InputHandler(const InputHandler& other) = delete;

        ~InputHandler();

        InputHandler& operator=(InputHandler&& other) = delete;
        InputHandler& operator=(const InputHandler& other) = delete;

        void Reset();
        void Update();

        virtual void Handle(const KeyEvent& key_event) override;
        virtual void Handle(const ButtonEvent& button_event) override;
        virtual void Handle(const CursorMovedEvent& cursor_moved_event) override;

        bool IsKeyPressed(unsigned int key);
        bool IsKeyJustPressed(unsigned int key);
        bool IsKeyReleased(unsigned int key);
        bool IsKeyJustReleased(unsigned int key);

        bool IsButtonPressed(unsigned int button);
        bool IsButtonJustPressed(unsigned int button);
        bool IsButtonReleased(unsigned int button);
        bool IsButtonJustReleased(unsigned int button);

        glm::vec2 GetCursorPosition();
        glm::vec2 GetCursorMovement();

    private:
        bool keys_pressed[GLFW_KEY_LAST + 1];
        bool keys_just_pressed[GLFW_KEY_LAST + 1];
        bool keys_just_released[GLFW_KEY_LAST + 1];

        bool buttons_pressed[GLFW_MOUSE_BUTTON_LAST + 1];
        bool buttons_just_pressed[GLFW_MOUSE_BUTTON_LAST + 1];
        bool buttons_just_released[GLFW_MOUSE_BUTTON_LAST + 1];

        glm::vec2 cursor_position;
        glm::vec2 cursor_movement;

        GLFWwindow* window;
    };
};

