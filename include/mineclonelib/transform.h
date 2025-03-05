#pragma once

#include <glm/glm.hpp>

namespace mc
{
class transform {
    public:
	transform() = default;
	~transform() = default;

	glm::vec3 right() const;
	glm::vec3 up() const;
	glm::vec3 forward() const;

	glm::mat4 get_matrix() const;

	inline glm::vec3 &position() noexcept
	{
		return m_position;
	}

	inline const glm::vec3 &position() const noexcept
	{
		return m_position;
	}

	inline glm::vec3 &rotation() noexcept
	{
		return m_rotation;
	}

	inline const glm::vec3 &rotation() const noexcept
	{
		return m_rotation;
	}

	inline glm::vec3 &scale() noexcept
	{
		return m_scale;
	}

	inline const glm::vec3 &scale() const noexcept
	{
		return m_scale;
	}

    private:
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
};
}
