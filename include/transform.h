#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Game
{
    class Transform
    {
    public:
        Transform();
        ~Transform();

        inline glm::vec3& Position() noexcept { return position; }
        inline const glm::vec3& Position() const noexcept { return position; }

        inline glm::quat& Rotation() noexcept { return rotation; }
        inline const glm::quat& Rotation() const noexcept { return rotation; }

        inline glm::vec3& Scale() noexcept { return scale; }
        inline const glm::vec3& Scale() const noexcept { return scale; }

        glm::vec3 Forward();
        glm::vec3 Up();
        glm::vec3 Right();

        glm::mat4 GetMatrix() const;

    private:
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };
};

