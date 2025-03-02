#include <mineclonelib/cvar.h>
#include <mineclonelib/misc.h>

#include <mineclonelib/io/input.h>
#include <mineclonelib/io/window.h>

#include <mineclonelib/render/context.h>

#include <glm/glm.hpp>

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
	render_api(mc::render::render_api::vulkan, "render/api",
		   "The render api to use");

int main()
{
	const char *app_name =
		mc::cvars<const char *>::get()->find("app/name")->get();

	const char *engine_name =
		mc::cvars<const char *>::get()->find("engine/name")->get();

	LOG_INFO(Default, "Starting app {} with engine {}", app_name,
		 engine_name);

	mc::window::init();

	std::optional<mc::window> window;
	window.emplace(window_title.get(), window_size.get().x,
		       window_size.get().y);

	std::optional<mc::input_manager> input;
	input.emplace();

	window->add_input_handler(&input.value());

	std::unique_ptr<mc::render::context> context =
		mc::render::context::create(&window.value());

	while (!window->should_close()) {
		input->preupdate();
		mc::window::poll_events();
		input->update();

		context->begin();
		context->present();
	}

	context.reset();

	window->remove_input_handler(&input.value());
	input.reset();

	window.reset();

	mc::window::terminate();
	return 0;
}
