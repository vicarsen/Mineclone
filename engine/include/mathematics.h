#pragma once

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace Math
{
    using namespace glm;

    template<typename Tp>
    inline bool Inside(const Tp& x, const Tp& min, const Tp& max)
    {
        return min <= x && x < max;
    }

    template<typename Tp>
    inline bool Inside(const tvec2<Tp>& v, const tvec2<Tp>& min, const tvec2<Tp>& max)
    {
        return Inside(v.x, min.x, max.x) && Inside(v.y, min.y, max.y);
    }

    template<typename Tp>
    inline bool Inside(const tvec3<Tp>& v, const tvec3<Tp>& min, const tvec3<Tp>& max)
    {
        return Inside(v.x, min.x, max.x) && Inside(v.y, min.y, max.y) && Inside(v.z, min.z, max.z);
    }

    template<typename Tp>
    inline bool Inside(const tvec4<Tp>& v, const tvec4<Tp>& min, const tvec4<Tp>& max)
    {
        return Inside(v.x, min.x, max.x) && Inside(v.y, min.y, max.y) && Inside(v.z, min.z, max.z) && Inside(v.w, min.w, max.w);
    }
};

