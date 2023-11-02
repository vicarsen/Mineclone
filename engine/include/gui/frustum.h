#pragma once

#include "../gui.h"
#include "../frustum.h"

namespace GUI
{
    namespace Math
    {
        void InputFrustum(const char* name, ::Math::Frustum& frustum);
        void InputFrustumPlane(const char* name, ::Math::Frustum::Plane& plane);
    };
};

