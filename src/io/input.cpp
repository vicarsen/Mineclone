#include "mineclonelib/io/input.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace mc
{
input_manager::input_manager()
{
	m_time = glfwGetTime();
}

void input_manager::preupdate()
{
	m_just_pressed.reset();
	m_just_released.reset();

	m_cursor_delta = { 0.0f, 0.0f };
	m_scroll = { 0.0f, 0.0f };
}

void input_manager::update()
{
	float time = glfwGetTime();
	m_delta_time = m_time - time;
	m_time = time;
}

void input_manager::key_callback(int key, input_event event)
{
	if (event == input_event::pressed) {
		if (!m_pressed.test(key)) {
			m_pressed.set(key);
			m_just_pressed.set(key);
		}
	} else {
		if (m_pressed.test(key)) {
			m_pressed.reset(key);
			m_just_released.set(key);
		}
	}
}

void input_manager::button_callback(int button, input_event event)
{
	if (event == input_event::pressed) {
		if (!m_pressed.test(button)) {
			m_pressed.set(button);
			m_just_pressed.set(button);
		}
	} else {
		if (m_pressed.test(button)) {
			m_pressed.reset(button);
			m_just_released.set(button);
		}
	}
}

void input_manager::cursor_callback(float x, float y)
{
	glm::vec2 cursor_pos = { x, y };
	m_cursor_delta = cursor_pos - m_cursor_pos;
	m_cursor_pos = cursor_pos;
}

void input_manager::scroll_callback(float offsetx, float offsety)
{
	m_scroll = { offsetx, offsety };
}
}
