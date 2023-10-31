#include "aabb.h"
#include "gui/aabb.h"
#include "format/aabb.h"

namespace GUI
{
    void InputAABB(const char* name, ::Math::AABB& aabb)
    {
        if(TreeNode(name))
        {
            InputFloat3("Min", &aabb.min[0]);
            InputFloat3("Max", &aabb.max[0]);

            TreePop();
        }
    }
};

