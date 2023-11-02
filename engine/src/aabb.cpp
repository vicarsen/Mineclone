#include "aabb.h"
#include "gui/aabb.h"
#include "format/aabb.h"

#include "gui/mathematics.h"

namespace GUI
{
    void InputAABB(const char* label, ::Math::AABB* aabb, const char* format, ImGuiInputTextFlags flags)
    {
        if(TreeNode(label))
        {
            InputVec3("Min", &aabb->min, format, flags);
            InputVec3("Max", &aabb->max, format, flags);

            TreePop();
        }
    }
};

