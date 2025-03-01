#include <mineclonelib/cvar.h>
#include <mineclonelib/misc.h>

#include <mineclonelib/io/window.h>

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

	while (!window->should_close()) {
		mc::window::poll_events();
	}

	window.reset();

	mc::window::terminate();
	return 0;
}
