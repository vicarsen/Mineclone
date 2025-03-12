#include "mineclonelib/application.h"
#include "mineclonelib/cvar.h"
#include "mineclonelib/misc.h"

#include "mineclonelib/render/render.h"

#include <condition_variable>
#include <glm/glm.hpp>
#include <memory>

#include <taskflow/algorithm/pipeline.hpp>
#include <taskflow/core/executor.hpp>
#include <taskflow/core/taskflow.hpp>

static mc::cvar<const char *> app_name("Mineclone", "app/name",
				       "The name of the application");

static mc::cvar<mc::version> app_version({ 0, 1, 0 }, "app/version",
					 "The version of the application");

static mc::cvar<const char *> engine_name("Mineclone Engine", "engine/name",
					  "The name of the engine");

static mc::cvar<mc::version> engine_version({ 0, 1, 0 }, "engine/version",
					    "The version of the engine");

static mc::cvar<const char *> window_title("Mineclone", "window/title",
					   "The title of the main window");

static mc::cvar<glm::ivec2> window_size({ 1280, 720 }, "window/size",
					"The size of the main window");

static mc::cvar<mc::render::render_api>
	render_api(mc::render::render_api::opengl, "render/api",
		   "The render api to use");

static mc::cvar<uint32_t> app_frames(
	2, "app/frames",
	"The maximum number of frames to generate in parallel i.e. the number of lines in the frame pipeline.");

namespace mc
{
application *application::s_instance = nullptr;

application::application()
{
	LOG_ASSERT(
		Default, s_instance == nullptr,
		"Application already created. Cannot instantiate more than one app");

	s_instance = this;
}

application::~application()
{
	s_instance = nullptr;
}

void application::run()
{
	const char *wnd_title = window_title.get();
	glm::ivec2 wnd_size = window_size.get();

	window::init();
	m_window = std::make_unique<window>(wnd_title, wnd_size.x, wnd_size.y);

	m_input = std::make_unique<input_manager>();
	m_window->add_input_handler(m_input.get());

	m_render_thread = std::make_unique<render_thread>();
	m_render_thread->init(m_window.get(), [&] { render(); });

	init();

	m_state.input.set_framebuffer(wnd_size);

	m_state.render.framebuffer = wnd_size;
	m_state.render.framebuffer_resized = false;

	m_state.render.view = glm::mat4(1.0f);
	m_state.render.projection = glm::mat4(1.0f);

	render::render_state render_state = m_state.render;

	std::mutex mutex;
	bool poll_events = false;
	bool start_update = false;
	std::condition_variable cv;

	tf::Taskflow taskflow;
	tf::Executor executor;

	auto start_pipe = [&](tf::Pipeflow &pf) {
		{
			std::lock_guard lock(mutex);
			poll_events = true;
		}

		cv.notify_one();

		{
			std::unique_lock lock(mutex);
			cv.wait(lock, [&] { return start_update; });

			start_update = false;
		}

		if (m_window->should_close()) {
			pf.stop();
		}
	};

	auto preupdate_pipe = [&](tf::Pipeflow &pf) {
		uint32_t frame = pf.line();
		preupdate(m_frames[frame]);
	};

	auto update_pipe = [&](tf::Pipeflow &pf) {
		uint32_t frame = pf.line();
		update(m_state, m_frames[frame]);
	};

	auto render_sync_pipe = [&](tf::Pipeflow &pf) {
		m_render_thread->sync(m_state.render);
	};

	tf::Pipeline pipeline(
		app_frames.get(),
		tf::Pipe{ tf::PipeType::SERIAL, std::move(start_pipe) },
		tf::Pipe{ tf::PipeType::PARALLEL, std::move(preupdate_pipe) },
		tf::Pipe{ tf::PipeType::SERIAL, std::move(update_pipe) },
		tf::Pipe{ tf::PipeType::SERIAL, std::move(render_sync_pipe) });

	tf::Task pipeline_task =
		taskflow.composed_of(pipeline).name("App Frames");

	tf::Future<void> done = executor.run(taskflow);

	m_render_thread->start();

	while (!m_window->should_close()) {
		// Wait for next frame to finish
		std::unique_lock lock(mutex);
		cv.wait(lock, [&] { return poll_events; });

		poll_events = false;

		// Poll events for next frame update
		m_input->preupdate(&m_state.input);
		window::poll_events();
		m_input->update();

		m_state.render.framebuffer = m_state.input.get_framebuffer();
		m_state.render.framebuffer_resized =
			m_state.input.is_framebuffer_resized();

		render_state = m_state.render;

		start_update = true;

		// Start next frame update
		lock.unlock();
		cv.notify_one();
	}

	done.wait();

	terminate();

	m_render_thread->terminate();
	m_render_thread.reset();

	m_window->remove_input_handler(m_input.get());
	m_input.reset();

	m_window.reset();
	window::terminate();
}
}
