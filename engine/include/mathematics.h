#pragma once

#include <cmath>

#include <glm/glm.hpp>

namespace Math
{
    template<typename Tp>
    inline bool Inside(const Tp& x, const Tp& min, const Tp& max)
    {
        return min <= x && x < max;
    }

    template<>
    bool Inside<glm::ivec3>(const glm::ivec3& v, const glm::ivec3& min, const glm::ivec3& max);

    template<>
    bool Inside<glm::vec3>(const glm::vec3& v, const glm::vec3& min, const glm::vec3& max);
};

