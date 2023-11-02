#pragma once

#include "mathematics.h"

namespace Math
{
    class Transform
    {
    public:
        Transform();
        ~Transform();

        inline vec3& Position() noexcept { return position; }
        inline const vec3& Position() const noexcept { return position; }

        inline quat& Rotation() noexcept { return rotation; }
        inline const quat& Rotation() const noexcept { return rotation; }

        inline vec3& Scale() noexcept { return scale; }
        inline const vec3& Scale() const noexcept { return scale; }

        vec3 Forward();
        vec3 Up();
        vec3 Right();

        mat4 GetMatrix() const;

    private:
        vec3 position;
        quat rotation;
        vec3 scale;
    };
};

