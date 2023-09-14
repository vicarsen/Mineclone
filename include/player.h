#pragma once

#include "logger.h"

#include "transform.h"

DECLARE_LOG_CATEGORY(Player);

namespace Game
{
    class Player
    {
    public:
        Player(float fov, float aspect_ratio);
        ~Player();

        inline Transform& GetTransform() noexcept { return transform; }
        inline const Transform& GetTransform() const noexcept { return transform; }

        inline glm::mat4& GetProjection() noexcept { return projection; }
        inline const glm::mat4& GetProjection() const noexcept { return projection; }

        void SetFOV(float fov);
        void SetAspectRatio(float aspect_ratio);

    private:
        Transform transform;
        glm::mat4 projection;
        float fov, aspect_ratio, near, far;
    };
};

