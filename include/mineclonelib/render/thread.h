#pragma once

#include "mineclonelib/io/window.h"
#include "mineclonelib/render/context.h"
#include "mineclonelib/render/gui.h"

#include "mineclonelib/render/render.h"
#include "mineclonelib/render/world.h"
#include <thread>

namespace mc
{
class render_thread {
    public:
	render_thread();
	~render_thread();

	void init(window *wnd, std::function<void()> callback);
	void start();
	void terminate();

	void sync(const render::render_state &state);

	inline render::context *get_context() const
	{
		return m_ctx.get();
	}

	inline render::gui_context *get_gui_context() const
	{
		return m_gui_ctx.get();
	}

	inline render::world_renderer *get_world_renderer() const
	{
		return m_world_renderer.get();
	}

    private:
	void run();

    private:
	std::thread m_thread;

	std::unique_ptr<render::context> m_ctx;
	std::unique_ptr<render::gui_context> m_gui_ctx;
	std::unique_ptr<render::world_renderer> m_world_renderer;

	render::render_state m_state;

	std::function<void()> m_callback;

	std::mutex m_mutex;
	std::condition_variable m_cv;
	bool m_trigger;
	bool m_sync;
	bool m_run;
};
}
