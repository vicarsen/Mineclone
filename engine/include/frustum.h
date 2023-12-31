// Based on https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644
#pragma once

#include "mathematics.h"
#include "aabb.h"

namespace Math
{
    class Frustum
    {
    public:
        enum class Plane
        {
            Left = 0,
            Right,
            Bottom,
            Top,
            Near,
            Far
        };

    public:
        Frustum(const mat4& matrix);

        bool IsAABBVisible(const AABB& aabb) const;

        inline const vec4& GetPlane(Plane plane) const noexcept { return planes[(int) plane]; }

    private:
        vec4 planes[6];
        vec3 corners[8];
    };
};

const char* ToString(::Math::Frustum::Plane plane);

