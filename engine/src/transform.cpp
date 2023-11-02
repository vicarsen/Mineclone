#include "transform.h"
#include "gui/transform.h"
#include "format/transform.h"

#include "gui/mathematics.h"

namespace Math
{
    Transform::Transform() :
        position(0.0f, 0.0f, 0.0f), rotation(vec3{ 0.0f, 0.0f, 0.0f }), scale(1.0f, 1.0f, 1.0f)
    {
    }

    Transform::~Transform()
    {
    }

    mat4 Transform::GetMatrix() const
    {
        mat4 translation = glm::translate(mat4(1.0f), this->position);
        mat4 rotation = glm::toMat4(this->rotation);
        mat4 scale = glm::scale(mat4(1.0f), this->scale);
        return translation * rotation * scale;
    }

    vec3 Transform::Forward()
    {
        return rotation * vec3(0.0f, 0.0f, -1.0f);
    }

    vec3 Transform::Up()
    {
        return rotation * vec3(0.0f, 1.0f, 0.0f);
    }

    vec3 Transform::Right()
    {
        return rotation * vec3(1.0f, 0.0f, 0.0f);
    }
};

namespace GUI
{
    namespace Math
    {
        void InputTransform(const char* label, ::Math::Transform* transform)
        {
            if(TreeNode(label))
            {
                ::Math::vec3 euler = ::Math::eulerAngles(transform->Rotation());

                DragVec3("Position", &transform->Position());
                DragVec3("Scale", &transform->Scale());
                DragVec3("Euler", &euler);

                TreePop();

                transform->Rotation() = euler;
            }
        }
    };
};

