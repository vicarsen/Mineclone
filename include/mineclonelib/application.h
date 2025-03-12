#pragma once

#include "mineclonelib/io/window.h"
#include "mineclonelib/io/input.h"

#include "mineclonelib/render/context.h"
#include "mineclonelib/render/gui.h"

#include "mineclonelib/render/render.h"
#include "mineclonelib/render/thread.h"
#include "mineclonelib/render/world.h"

#include <memory>

namespace mc
{
struct application_frame {};

struct application_state {
	input_state input;
	render::render_state render;
};

class application {
    public:
	application();
	virtual ~application();

	virtual void init()
	{
	}

	virtual void preupdate(application_frame &frame)
	{
	}

	virtual void update(application_state &state, application_frame &frame)
	{
	}

	virtual void render()
	{
	}

	virtual void terminate()
	{
	}

	void run();

	inline window *get_window() const
	{
		return m_window.get();
	}

	inline input_manager *get_input() const
	{
		return m_input.get();
	}

	inline render_thread *get_render_thread() const
	{
		return m_render_thread.get();
	}

	static inline application *get() noexcept
	{
		return s_instance;
	}

    private:
	static application *s_instance;

	std::unique_ptr<window> m_window;
	std::unique_ptr<input_manager> m_input;

	std::unique_ptr<render_thread> m_render_thread;

	application_state m_state;
	std::vector<application_frame> m_frames;
};
}
