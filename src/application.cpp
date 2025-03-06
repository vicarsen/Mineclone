#include "mineclonelib/application.h"
#include "mineclonelib/cvar.h"
#include "mineclonelib/misc.h"

#include "mineclonelib/render/context.h"
#include "mineclonelib/render/gui.h"
#include "mineclonelib/render/world.h"

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

	m_render_ctx = render::context::create(m_window.get());
	m_gui_ctx = render::gui_context::create(m_render_ctx.get());

	m_world_renderer = render::world_renderer::create(m_render_ctx.get());

	init();

	m_render_ctx->unmake_current();

	m_frames.resize(app_frames.get());

	m_frames.back().input.set_framebuffer(wnd_size);

	m_frames.back().render.framebuffer = wnd_size;
	m_frames.back().render.framebuffer_resized = false;

	m_frames.back().render.view = glm::mat4(1.0f);
	m_frames.back().render.projection = glm::mat4(1.0f);

	std::mutex mutex;
	uint32_t current_frame;
	bool should_poll = false;
	bool polled = false;
	std::condition_variable cv;

	tf::Taskflow taskflow;
	tf::Executor executor;

	auto start_pipe = [&](tf::Pipeflow &pf) {
		{
			std::lock_guard lock(mutex);

			current_frame = pf.line();
			should_poll = true;
		}

		cv.notify_one();

		{
			std::unique_lock lock(mutex);
			cv.wait(lock, [&] { return polled; });

			polled = false;
		}

		if (m_window->should_close()) {
			pf.stop();
		}
	};

	auto update_pipe = [&](tf::Pipeflow &pf) {
		uint32_t frame = pf.line();
		update(m_frames[frame]);
	};

	auto render_pipe = [&](tf::Pipeflow &pf) {
		uint32_t frame = pf.line();

		m_render_ctx->make_current();

		m_render_ctx->begin(&m_frames[frame].render);
		m_gui_ctx->begin();

		render();

		m_world_renderer->render(m_frames[frame].render.view,
					 m_frames[frame].render.projection);

		m_gui_ctx->present();
		m_render_ctx->present();

		m_render_ctx->unmake_current();
	};

	tf::Pipeline pipeline(
		app_frames.get(),
		tf::Pipe{ tf::PipeType::SERIAL, std::move(start_pipe) },
		tf::Pipe{ tf::PipeType::PARALLEL, std::move(update_pipe) },
		tf::Pipe{ tf::PipeType::SERIAL, std::move(render_pipe) });

	tf::Task pipeline_task =
		taskflow.composed_of(pipeline).name("App Frames");

	tf::Future<void> done = executor.run(taskflow);

	while (!m_window->should_close()) {
		std::unique_lock lock(mutex);
		cv.wait(lock, [&] { return should_poll; });

		uint32_t prev_frame =
			(current_frame == 0 ? m_frames.size() : current_frame) -
			1;

		m_frames[current_frame].input = m_frames[prev_frame].input;
		m_input->preupdate(&m_frames[current_frame].input);
		window::poll_events();
		m_input->update();

		m_frames[current_frame].render.framebuffer =
			m_frames[current_frame].input.get_framebuffer();

		m_frames[current_frame].render.framebuffer_resized =
			m_frames[current_frame].input.is_framebuffer_resized();

		should_poll = false;
		polled = true;

		lock.unlock();
		cv.notify_one();
	}

	done.wait();

	m_frames.clear();

	terminate();

	m_world_renderer.reset();

	m_gui_ctx.reset();
	m_render_ctx.reset();

	m_window->remove_input_handler(m_input.get());
	m_input.reset();

	m_window.reset();
	window::terminate();
}
}
