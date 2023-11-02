#include "transform.h"

#include <glm/gtx/transform.hpp>

namespace Math
{
    Transform::Transform() :
        position(0.0f, 0.0f, 0.0f), rotation(glm::vec3{ 0.0f, 0.0f, 0.0f }), scale(1.0f, 1.0f, 1.0f)
    {
    }

    Transform::~Transform()
    {
    }

    glm::mat4 Transform::GetMatrix() const
    {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), this->position);
        glm::mat4 rotation = glm::toMat4(this->rotation);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), this->scale);
        return translation * rotation * scale;
    }

    glm::vec3 Transform::Forward()
    {
        return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::vec3 Transform::Up()
    {
        return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::vec3 Transform::Right()
    {
        return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }
};

