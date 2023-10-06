#include "mathematics.h"

namespace Math
{
    template<>
    bool Inside<glm::ivec3>(const glm::ivec3& x, const glm::ivec3& min, const glm::ivec3& max)
    {
        return Inside(x.x, min.x, max.x) && Inside(x.y, min.y, max.y) && Inside(x.z, min.z, max.z);
    }

    template<>
    bool Inside<glm::vec3>(const glm::vec3& x, const glm::vec3& min, const glm::vec3& max)
    {
        return Inside(x.x, min.x, max.x) && Inside(x.y, min.y, max.y) && Inside(x.z, min.z, max.z);
    }
};

