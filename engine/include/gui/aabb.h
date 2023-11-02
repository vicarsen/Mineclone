#pragma once

#include "../gui.h"

#include "../aabb.h"

namespace GUI
{
    void InputAABB(const char* name, ::Math::AABB& aabb, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
};

