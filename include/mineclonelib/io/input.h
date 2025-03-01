#pragma once

#include <bitset>
#include <glm/glm.hpp>

#include "mineclonelib/io/keys.h"

namespace mc
{
class input_handler {
    public:
	virtual void key_callback(int key, input_event event)
	{
	}
	virtual void button_callback(int button, input_event event)
	{
	}
	virtual void cursor_callback(float x, float y)
	{
	}
	virtual void scroll_callback(float offsetx, float offsety)
	{
	}
};

class input_manager : public input_handler {
    public:
	input_manager();
	~input_manager() = default;

	void preupdate();
	void update();

	inline bool is_key_pressed(int key) const noexcept
	{
		return m_pressed.test(key);
	}

	inline bool is_key_just_pressed(int key) const noexcept
	{
		return m_just_pressed.test(key);
	}

	inline bool is_key_just_released(int key) const noexcept
	{
		return m_just_released.test(key);
	}

	inline bool is_button_pressed(int key) const noexcept
	{
		return m_pressed.test(key);
	}

	inline bool is_button_just_pressed(int key) const noexcept
	{
		return m_just_pressed.test(key);
	}

	inline bool is_button_just_released(int key) const noexcept
	{
		return m_just_released.test(key);
	}

	inline glm::vec2 get_cursor_pos() const noexcept
	{
		return m_cursor_pos;
	}

	inline glm::vec2 get_cursor_delta() const noexcept
	{
		return m_cursor_delta;
	}

	inline glm::vec2 get_scroll() const noexcept
	{
		return m_scroll;
	}

	inline float get_time() const noexcept
	{
		return m_time;
	}

	inline float get_delta_time() const noexcept
	{
		return m_delta_time;
	}

	void key_callback(int key, input_event event) override;
	void button_callback(int button, input_event event) override;
	void cursor_callback(float x, float y) override;
	void scroll_callback(float offsetx, float offsety) override;

    private:
	std::bitset<MC_KEY_LAST + 1> m_pressed;
	std::bitset<MC_KEY_LAST + 1> m_just_pressed;
	std::bitset<MC_KEY_LAST + 1> m_just_released;

	glm::vec2 m_cursor_pos, m_cursor_delta;
	glm::vec2 m_scroll;

	float m_time, m_delta_time;
};
}
