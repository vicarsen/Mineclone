#include "mineclonelib/transform.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <glm/gtc/quaternion.hpp>

namespace mc
{
glm::vec3 transform::right() const
{
	return glm::quat(m_rotation) * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 transform::up() const
{
	return glm::quat(m_rotation) * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 transform::forward() const
{
	return glm::quat(m_rotation) * glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::mat4 transform::get_matrix() const
{
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), m_position);
	glm::mat4 rotate = glm::mat4_cast(glm::quat(m_rotation));
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);
	return translate * rotate * scale;
}
}
