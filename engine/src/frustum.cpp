#include "frustum.h"
#include "gui/frustum.h"
#include "format/frustum.h"

#include "utils/type.h"

namespace Math
{
    static inline ::Utils::USize IndexOf(Frustum::Plane a, Frustum::Plane b)
    {
        return (::Utils::USize) a * (9 - (::Utils::USize) a) / 2 + (int) b - 1;
    }

    static inline vec3 Intersection(Frustum::Plane a, Frustum::Plane b, Frustum::Plane c, const vec4* planes, const vec3* crosses)
    {
        float d = glm::dot(vec3(planes[(int) a]), crosses[IndexOf(b, c)]);
        vec3 res = mat3(crosses[IndexOf(b, c)], -crosses[IndexOf(a, c)], crosses[IndexOf(a, b)]) *
                   vec3(planes[(int) a].w, planes[(int) b].w, planes[(int) c].w);
        return res * (-1.0f / d);
    }

    Frustum::Frustum(const mat4& matrix)
    {
        mat4 mat = glm::transpose(matrix);
        planes[(::Utils::USize) Plane::Left]   = mat[3] + mat[0];
        planes[(::Utils::USize) Plane::Right]  = mat[3] - mat[0];
        planes[(::Utils::USize) Plane::Bottom] = mat[3] + mat[1];
        planes[(::Utils::USize) Plane::Top]    = mat[3] - mat[1];
        planes[(::Utils::USize) Plane::Near]   = mat[3] + mat[2];
        planes[(::Utils::USize) Plane::Far]    = mat[3] - mat[2];

        vec3 crosses[] =
        {
            cross(vec3(planes[(::Utils::USize) Plane::Left]), vec3(planes[(::Utils::USize) Plane::Right])),
            cross(vec3(planes[(::Utils::USize) Plane::Left]), vec3(planes[(::Utils::USize) Plane::Bottom])),
            cross(vec3(planes[(::Utils::USize) Plane::Left]), vec3(planes[(::Utils::USize) Plane::Top])),
            cross(vec3(planes[(::Utils::USize) Plane::Left]), vec3(planes[(::Utils::USize) Plane::Near])),
            cross(vec3(planes[(::Utils::USize) Plane::Left]), vec3(planes[(::Utils::USize) Plane::Far])),
            cross(vec3(planes[(::Utils::USize) Plane::Right]), vec3(planes[(::Utils::USize) Plane::Bottom])),
            cross(vec3(planes[(::Utils::USize) Plane::Right]), vec3(planes[(::Utils::USize) Plane::Top])),
            cross(vec3(planes[(::Utils::USize) Plane::Right]), vec3(planes[(::Utils::USize) Plane::Near])),
            cross(vec3(planes[(::Utils::USize) Plane::Right]), vec3(planes[(::Utils::USize) Plane::Far])),
            cross(vec3(planes[(::Utils::USize) Plane::Bottom]), vec3(planes[(::Utils::USize) Plane::Top])),
            cross(vec3(planes[(::Utils::USize) Plane::Bottom]), vec3(planes[(::Utils::USize) Plane::Near])),
            cross(vec3(planes[(::Utils::USize) Plane::Bottom]), vec3(planes[(::Utils::USize) Plane::Far])),
            cross(vec3(planes[(::Utils::USize) Plane::Top]), vec3(planes[(::Utils::USize) Plane::Near])),
            cross(vec3(planes[(::Utils::USize) Plane::Top]), vec3(planes[(::Utils::USize) Plane::Far])),
            cross(vec3(planes[(::Utils::USize) Plane::Near]), vec3(planes[(::Utils::USize) Plane::Far]))
        };

        corners[0] = Intersection(Plane::Left, Plane::Bottom, Plane::Near, planes, crosses);
        corners[1] = Intersection(Plane::Left, Plane::Top, Plane::Near, planes, crosses);
        corners[2] = Intersection(Plane::Right, Plane::Bottom, Plane::Near, planes, crosses);
        corners[3] = Intersection(Plane::Right, Plane::Top, Plane::Near, planes, crosses);
        corners[4] = Intersection(Plane::Left, Plane::Bottom, Plane::Far, planes, crosses);
        corners[5] = Intersection(Plane::Left, Plane::Top, Plane::Far, planes, crosses);
        corners[6] = Intersection(Plane::Right, Plane::Bottom, Plane::Far, planes, crosses);
        corners[7] = Intersection(Plane::Right, Plane::Top, Plane::Far, planes, crosses);
    }

    bool Frustum::IsAABBVisible(const AABB& aabb) const
    {
        for(::Utils::USize i = 0; i < 6; i++)
        {
            if((glm::dot(planes[i], vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], vec4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], vec4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], vec4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], vec4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], vec4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], vec4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f)) < 0.0f) &&
               (glm::dot(planes[i], vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f)) < 0.0f))
            {
                return false;
            }
        }

        int out;
        out = 0; for(::Utils::USize i = 0; i < 8; i++) out += (corners[i].x > aabb.max.x ? 1 : 0); if(out == 8) return false;
        out = 0; for(::Utils::USize i = 0; i < 8; i++) out += (corners[i].x < aabb.min.x ? 1 : 0); if(out == 8) return false;
        out = 0; for(::Utils::USize i = 0; i < 8; i++) out += (corners[i].y > aabb.max.y ? 1 : 0); if(out == 8) return false;
        out = 0; for(::Utils::USize i = 0; i < 8; i++) out += (corners[i].y < aabb.min.y ? 1 : 0); if(out == 8) return false;
        out = 0; for(::Utils::USize i = 0; i < 8; i++) out += (corners[i].z > aabb.max.z ? 1 : 0); if(out == 8) return false;
        out = 0; for(::Utils::USize i = 0; i < 8; i++) out += (corners[i].z < aabb.min.z ? 1 : 0); if(out == 8) return false;

        return true;
    }
};

namespace GUI
{
    namespace Math
    {
        void InputFrustumPlane(const char* name, ::Math::Frustum::Plane* plane, ImGuiSelectableFlags flags, const ImVec2& size_arg)
        {
            if(BeginCombo(name, ToString(*plane)))
            {
                for(int i = 0; i < 6; i++)
                {
                    bool is_selected = (*plane == (::Math::Frustum::Plane) i);
                    if(Selectable(ToString((::Math::Frustum::Plane) i), is_selected))
                        is_selected = true;
                    if(is_selected)
                        SetItemDefaultFocus();
                }

                EndCombo();
            }
        }
    };
};

const char* ToString(::Math::Frustum::Plane plane)
{
    switch(plane)
    {
    case ::Math::Frustum::Plane::Left: return "Left";
    case ::Math::Frustum::Plane::Right: return "Right";
    case ::Math::Frustum::Plane::Bottom: return "Bottom";
    case ::Math::Frustum::Plane::Top: return "Top";
    case ::Math::Frustum::Plane::Near: return "Near";
    case ::Math::Frustum::Plane::Far: return "Far";
    default: return "";
    }
}

