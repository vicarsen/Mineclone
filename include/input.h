#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "logger.h"

DECLARE_LOG_CATEGORY(Logger);

namespace Input
{
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

    void Init();
    void Destroy();

    void PreUpdate();
    void Update();
};

