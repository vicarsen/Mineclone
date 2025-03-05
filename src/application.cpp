#include "mineclonelib/application.h"
#include "mineclonelib/cvar.h"
#include "mineclonelib/misc.h"

#include "mineclonelib/render/context.h"
#include "mineclonelib/render/gui.h"
#include "mineclonelib/render/world.h"
#include <glm/glm.hpp>
#include <memory>

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

	while (!m_window->should_close()) {
		m_input->preupdate();
		window::poll_events();
		m_input->update();

		update();

		m_render_ctx->begin();
		m_gui_ctx->begin();

		render();

		m_world_renderer->render(m_view, m_projection);

		m_gui_ctx->present();
		m_render_ctx->present();
	}

	terminate();

	m_world_renderer.reset();

	m_gui_ctx.reset();
	m_render_ctx.reset();

	m_window->remove_input_handler(m_input.get());
	m_input.reset();

	m_window.reset();
	window::terminate();
}

void application::set_view(const glm::mat4 &view)
{
	m_view = view;
}

void application::set_projection(const glm::mat4 &projection)
{
	m_projection = projection;
}
}
