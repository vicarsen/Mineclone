#include "frustum.h"

namespace Math
{
    std::size_t IndexOf(Frustum::Planes a, Frustum::Planes b)
    {
        return a * (9 - a) / 2 + b - 1;
    }

    glm::vec3 Intersection(Frustum::Planes a, Frustum::Planes b, Frustum::Planes c, const glm::vec4* planes, const glm::vec3* crosses)
    {
        float d = glm::dot(glm::vec3(planes[a]), crosses[IndexOf(b, c)]);
        glm::vec3 res = glm::mat3(crosses[IndexOf(b, c)], -crosses[IndexOf(a, c)], crosses[IndexOf(a, b)]) *
                        glm::vec3(planes[a].w, planes[b].w, planes[c].w);
        return res * (-1.0f / d);
    }

    Frustum::Frustum(const glm::mat4& matrix)
    {
        glm::mat4 mat = glm::transpose(matrix);
        planes[Left]   = mat[3] + mat[0];
        planes[Right]  = mat[3] - mat[0];
        planes[Bottom] = mat[3] + mat[1];
        planes[Top]    = mat[3] - mat[1];
        planes[Near]   = mat[3] + mat[2];
        planes[Far]    = mat[3] - mat[2];

        glm::vec3 crosses[] =
        {
            glm::cross(glm::vec3(planes[Left]), glm::vec3(planes[Right])),
            glm::cross(glm::vec3(planes[Left]), glm::vec3(planes[Bottom])),
            glm::cross(glm::vec3(planes[Left]), glm::vec3(planes[Top])),
            glm::cross(glm::vec3(planes[Left]), glm::vec3(planes[Near])),
            glm::cross(glm::vec3(planes[Left]), glm::vec3(planes[Far])),
            glm::cross(glm::vec3(planes[Right]), glm::vec3(planes[Bottom])),
            glm::cross(glm::vec3(planes[Right]), glm::vec3(planes[Top])),
            glm::cross(glm::vec3(planes[Right]), glm::vec3(planes[Near])),
            glm::cross(glm::vec3(planes[Right]), glm::vec3(planes[Far])),
            glm::cross(glm::vec3(planes[Bottom]), glm::vec3(planes[Top])),
            glm::cross(glm::vec3(planes[Bottom]), glm::vec3(planes[Near])),
            glm::cross(glm::vec3(planes[Bottom]), glm::vec3(planes[Far])),
            glm::cross(glm::vec3(planes[Top]), glm::vec3(planes[Near])),
            glm::cross(glm::vec3(planes[Top]), glm::vec3(planes[Far])),
            glm::cross(glm::vec3(planes[Near]), glm::vec3(planes[Far]))
        };

        corners[0] = Intersection(Left, Bottom, Near, planes, crosses);
        corners[1] = Intersection(Left, Top, Near, planes, crosses);
        corners[2] = Intersection(Right, Bottom, Near, planes, crosses);
        corners[3] = Intersection(Right, Top, Near, planes, crosses);
        corners[4] = Intersection(Left, Bottom, Far, planes, crosses);
        corners[5] = Intersection(Left, Top, Far, planes, crosses);
        corners[6] = Intersection(Right, Bottom, Far, planes, crosses);
        corners[7] = Intersection(Right, Top, Far, planes, crosses);
    }

    bool Frustum::IsAABBVisible(const AABB& aabb) const
    {
        for(int i = 0; i < 6; i++)
        {
            if((glm::dot(planes[i], glm::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], glm::vec4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], glm::vec4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], glm::vec4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], glm::vec4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], glm::vec4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], glm::vec4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], glm::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f)) < 0.0f))
            {
                return false;
            }
        }

        int out;
        out = 0; for(int i = 0; i < 8; i++) out += (corners[i].x > aabb.max.x ? 1 : 0); if(out == 8) return false;
        out = 0; for(int i = 0; i < 8; i++) out += (corners[i].x < aabb.min.x ? 1 : 0); if(out == 8) return false;
        out = 0; for(int i = 0; i < 8; i++) out += (corners[i].y > aabb.max.y ? 1 : 0); if(out == 8) return false;
        out = 0; for(int i = 0; i < 8; i++) out += (corners[i].y < aabb.min.y ? 1 : 0); if(out == 8) return false;
        out = 0; for(int i = 0; i < 8; i++) out += (corners[i].z > aabb.max.z ? 1 : 0); if(out == 8) return false;
        out = 0; for(int i = 0; i < 8; i++) out += (corners[i].z < aabb.min.z ? 1 : 0); if(out == 8) return false;

        return true;
    }
};

