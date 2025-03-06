#include "mineclonelib/io/input.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace mc
{
input_manager::input_manager()
{
}

void input_manager::preupdate(input_state *state)
{
	m_state = state;

	m_state->m_just_pressed.reset();
	m_state->m_just_released.reset();

	m_state->m_cursor_delta = { 0.0f, 0.0f };
	m_state->m_scroll = { 0.0f, 0.0f };

	m_state->m_framebuffer_resized = false;
}

void input_manager::update()
{
	float time = glfwGetTime();
	m_state->m_delta_time = time - m_state->m_time;
	m_state->m_time = time;
}

void input_manager::key_callback(int key, input_event event)
{
	if (event == input_event::pressed) {
		if (!m_state->m_pressed.test(key)) {
			m_state->m_pressed.set(key);
			m_state->m_just_pressed.set(key);
		}
	} else {
		if (m_state->m_pressed.test(key)) {
			m_state->m_pressed.reset(key);
			m_state->m_just_released.set(key);
		}
	}
}

void input_manager::button_callback(int button, input_event event)
{
	if (event == input_event::pressed) {
		if (!m_state->m_pressed.test(button)) {
			m_state->m_pressed.set(button);
			m_state->m_just_pressed.set(button);
		}
	} else {
		if (m_state->m_pressed.test(button)) {
			m_state->m_pressed.reset(button);
			m_state->m_just_released.set(button);
		}
	}
}

void input_manager::cursor_callback(float x, float y)
{
	glm::vec2 cursor_pos = { x, y };
	m_state->m_cursor_delta = cursor_pos - m_state->m_cursor_pos;
	m_state->m_cursor_pos = cursor_pos;
}

void input_manager::scroll_callback(float offsetx, float offsety)
{
	m_state->m_scroll = { offsetx, offsety };
}

void input_manager::framebuffer_callback(int width, int height)
{
	m_state->m_framebuffer = { width, height };
	m_state->m_framebuffer_resized = true;
}
}
