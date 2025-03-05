#include "glm/ext/matrix_transform.hpp"
#include <mineclonelib/cvar.h>
#include <mineclonelib/misc.h>
#include <mineclonelib/transform.h>

#include <mineclonelib/io/input.h>
#include <mineclonelib/io/window.h>

#include <mineclonelib/world/chunk.h>

#include <mineclonelib/render/context.h>
#include <mineclonelib/render/gui.h>
#include <mineclonelib/render/world.h>

#include <glm/gtc/matrix_transform.hpp>

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

	std::unique_ptr<mc::render::gui_context> gui_ctx =
		mc::render::gui_context::create(context.get());

	std::unique_ptr<mc::world::chunk_draw_data_generator> chunk_gen =
		std::make_unique<mc::world::simple_chunk_draw_data_generator>();

	mc::world::chunk chunk;

	for (int i = 1; i <= 63; i++) {
		for (int j = 1; j <= 63; j++) {
			for (int k = 1; k <= 63; k++) {
				int dx = (i - 32) * (i - 32);
				int dy = (j - 32) * (j - 32);
				int dz = (k - 32) * (k - 32);

				if (dx + dy + dz <= 23 * 23) {
					chunk.set(i, j, k,
						  mc::world::blocks::dirt);
				}
			}
		}
	}

	mc::world::chunk_draw_data draw_data = chunk_gen->generate(&chunk);

	std::unique_ptr<mc::render::world_renderer> world_renderer =
		mc::render::world_renderer::create(context.get());

	const int dim = 16;
	mc::render::chunk_handle chunks[dim][dim];
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			chunks[i][j] = world_renderer->alloc_chunk();
		}
	}

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			glm::mat4 model = glm::translate(
				glm::mat4(1.0f),
				glm::vec3(i * CHUNK_SIZE, 0, j * CHUNK_SIZE));

			world_renderer->upload_chunk(chunks[i][j], &draw_data,
						     model);
		}
	}

	mc::transform camera;

	float sensitivity = 0.005f;
	float speed = 5.0f;

	window->set_cursor(mc::cursor_mode::hidden);

	while (!window->should_close()) {
		input->preupdate();
		mc::window::poll_events();
		input->update();

		if (input->is_key_just_pressed(MC_KEY_ESCAPE)) {
			window->toggle_cursor();
		}

		if (window->get_cursor() == mc::cursor_mode::hidden) {
			float scroll = input->get_scroll().y;

			if (scroll > 0) {
				speed *= 1.0f + 1.0f / scroll;
			} else if (scroll < 0) {
				speed /= 1.0f - 1.0f / scroll;
			}

			camera.rotation().x = glm::clamp(
				camera.rotation().x -
					input->get_cursor_delta().y *
						sensitivity,
				glm::radians(-89.0f), glm::radians(89.0f));

			camera.rotation().y -=
				input->get_cursor_delta().x * sensitivity;

			glm::vec3 right = camera.right();
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 forward = camera.forward();

			camera.position() -=
				static_cast<float>(
					(input->is_key_pressed(MC_KEY_D) -
					 input->is_key_pressed(MC_KEY_A))) *
				speed * right * input->get_delta_time();

			camera.position() -=
				static_cast<float>(
					(input->is_key_pressed(MC_KEY_SPACE) -
					 input->is_key_pressed(
						 MC_KEY_LEFT_SHIFT))) *
				speed * up * input->get_delta_time();

			camera.position() +=
				static_cast<float>(
					input->is_key_pressed(MC_KEY_W) -
					input->is_key_pressed(MC_KEY_S)) *
				speed * forward * input->get_delta_time();
		}

		glm::ivec2 size = window_size.get();
		float aspect = static_cast<float>(size.x) / size.y;
		glm::mat4 projection = glm::perspective(glm::radians(90.0f),
							aspect, 0.1f, 1000.0f);

		context->begin();
		gui_ctx->begin();

		world_renderer->render(glm::inverse(camera.get_matrix()),
				       projection);

		gui_ctx->present();
		context->present();
	}

	world_renderer.reset();

	chunk_gen.reset();

	gui_ctx.reset();
	context.reset();

	window->remove_input_handler(&input.value());
	input.reset();

	window.reset();

	mc::window::terminate();
	return 0;
}
