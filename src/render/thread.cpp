#include "mineclonelib/render/context.h"
#include "mineclonelib/render/gui.h"
#include "mineclonelib/render/world.h"
#include <mineclonelib/render/thread.h>
#include <mutex>
#include <thread>

namespace mc
{
render_thread::render_thread()
{
}

render_thread::~render_thread()
{
}

void render_thread::init(window *wnd, std::function<void()> callback)
{
	m_ctx = render::context::create(wnd);
	m_gui_ctx = render::gui_context::create(m_ctx.get());
	m_world_renderer = render::world_renderer::create(m_ctx.get());

	m_callback = callback;
}

void render_thread::start()
{
	m_ctx->unmake_current();

	m_trigger = false;
	m_sync = true;
	m_run = true;

	m_thread = std::thread(&render_thread::run, this);
}

void render_thread::terminate()
{
	{
		std::lock_guard lock(m_mutex);
		m_trigger = true;
		m_run = false;
	}

	m_cv.notify_one();

	m_thread.join();

	m_ctx->make_current();

	m_world_renderer.reset();
	m_gui_ctx.reset();
	m_ctx.reset();
}

void render_thread::sync(const render::render_state &state)
{
	{
		std::unique_lock lock(m_mutex);

		m_cv.wait(lock, [&] { return m_sync; });
		m_sync = false;
	}

	{
		std::lock_guard lock(m_mutex);
		m_state = state;
		m_trigger = true;
	}

	m_cv.notify_one();
}

void render_thread::run()
{
	m_ctx->make_current();

	while (true) {
		// Wait for trigger
		std::unique_lock lock(m_mutex);

		m_cv.wait(lock, [&] { return m_trigger; });
		m_trigger = false;

		if (!m_run) {
			break;
		}

		// Render current frame in parallel
		m_ctx->begin(&m_state);
		m_gui_ctx->begin();

		m_callback();

		m_world_renderer->render(m_state.view, m_state.projection);

		m_gui_ctx->present();
		m_ctx->present();

		// Sync
		m_sync = true;

		lock.unlock();
		m_cv.notify_one();
	}

	m_ctx->unmake_current();
}
}
