#pragma once

#include "../gui.h"

#include "../frustum.h"

namespace GUI
{
    namespace Math
    {
        void InputFrustumPlane(const char* name, ::Math::Frustum::Plane* plane, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));
    };
};

