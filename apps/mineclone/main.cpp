#include <mineclonelib/cvar.h>
#include <mineclonelib/misc.h>

#include <mineclonelib/io/input.h>
#include <mineclonelib/io/window.h>

#include <glm/glm.hpp>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

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

	glfwMakeContextCurrent(window->get_handle());

	LOG_ASSERT(Default, gladLoadGL((GLADloadfunc)glfwGetProcAddress),
		   "Failed to load OpenGL");

	std::optional<mc::input_manager> input;
	input.emplace();

	window->add_input_handler(&input.value());

	while (!window->should_close()) {
		input->preupdate();
		mc::window::poll_events();
		input->update();

		LOG_INFO_IF(Default, input->is_key_pressed(MC_KEY_S),
			    "S is pressed");
		LOG_INFO_IF(Default, input->is_key_just_pressed(MC_KEY_A),
			    "A was just pressed");
		LOG_INFO_IF(Default, input->is_key_just_released(MC_KEY_D),
			    "D was just released");

		LOG_INFO_IF(Default,
			    input->is_button_pressed(MC_MOUSE_BUTTON_MIDDLE),
			    "Middle mouse button is pressed");
		LOG_INFO_IF(Default,
			    input->is_key_just_pressed(MC_MOUSE_BUTTON_LEFT),
			    "Left mouse button was just pressed");
		LOG_INFO_IF(Default,
			    input->is_key_just_released(MC_MOUSE_BUTTON_RIGHT),
			    "Right mouse button was just released");

		glm::vec2 cursor_delta = input->get_cursor_delta();
		if (cursor_delta.x != 0 || cursor_delta.y != 0) {
			LOG_INFO(Default, "Mouse was moved by {} {}",
				 cursor_delta.x, cursor_delta.y);
		}

		glm::vec2 scroll = input->get_scroll();
		if (scroll.x != 0 || scroll.y != 0) {
			LOG_INFO(Default, "Scrolled by {} {}", scroll.x,
				 scroll.y);
		}

		glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window->get_handle());
	}

	window->remove_input_handler(&input.value());
	input.reset();

	window.reset();

	mc::window::terminate();
	return 0;
}
