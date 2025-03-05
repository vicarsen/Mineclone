#include "imgui.h"
#include <mineclonelib/mineclone.h>

#include <glm/gtc/matrix_transform.hpp>

class mineclone_application : public mc::application {
    public:
	mineclone_application()
	{
	}

	virtual ~mineclone_application()
	{
	}

	virtual void init() override
	{
		std::unique_ptr<mc::world::chunk_draw_data_generator> chunk_gen =
			std::make_unique<
				mc::world::simple_chunk_draw_data_generator>();

		mc::world::chunk chunk;

		for (int i = 1; i <= 63; i++) {
			for (int j = 1; j <= 63; j++) {
				for (int k = 1; k <= 63; k++) {
					int dx = (i - 32) * (i - 32);
					int dy = (j - 32) * (j - 32);
					int dz = (k - 32) * (k - 32);

					if (dx + dy + dz <= 23 * 23) {
						chunk.set(
							i, j, k,
							mc::world::blocks::dirt);
					}
				}
			}
		}

		mc::world::chunk_draw_data draw_data =
			chunk_gen->generate(&chunk);

		mc::render::world_renderer *world_renderer =
			get_world_renderer();

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
					glm::vec3(i * CHUNK_SIZE, 0,
						  j * CHUNK_SIZE));

				world_renderer->upload_chunk(chunks[i][j],
							     &draw_data, model);
			}
		}

		get_window()->set_cursor(mc::cursor_mode::hidden);
	}

	virtual void update() override
	{
		mc::input_manager *input = get_input();
		mc::window *window = get_window();

		if (input->is_key_just_pressed(MC_KEY_ESCAPE)) {
			window->toggle_cursor();
		}

		if (window->get_cursor() == mc::cursor_mode::hidden) {
			float scroll = input->get_scroll().y;

			if (scroll > 0) {
				m_speed *= 1.0f + 1.0f / scroll;
			} else if (scroll < 0) {
				m_speed /= 1.0f - 1.0f / scroll;
			}

			m_camera.rotation().x = glm::clamp(
				m_camera.rotation().x -
					input->get_cursor_delta().y *
						m_sensitivity,
				glm::radians(-89.0f), glm::radians(89.0f));

			m_camera.rotation().y -=
				input->get_cursor_delta().x * m_sensitivity;

			glm::vec3 right = m_camera.right();
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 forward = m_camera.forward();

			m_camera.position() +=
				static_cast<float>(
					(input->is_key_pressed(MC_KEY_D) -
					 input->is_key_pressed(MC_KEY_A))) *
				m_speed * right * input->get_delta_time();

			m_camera.position() +=
				static_cast<float>(
					(input->is_key_pressed(MC_KEY_SPACE) -
					 input->is_key_pressed(
						 MC_KEY_LEFT_SHIFT))) *
				m_speed * up * input->get_delta_time();

			m_camera.position() -=
				static_cast<float>(
					input->is_key_pressed(MC_KEY_W) -
					input->is_key_pressed(MC_KEY_S)) *
				m_speed * forward * input->get_delta_time();
		}

		glm::ivec2 size = mc::cvars<glm::ivec2>::get()->find_or(
			"window/size", {});

		float aspect = static_cast<float>(size.x) / size.y;
		glm::mat4 projection = glm::perspective(glm::radians(90.0f),
							aspect, 0.1f, 1000.0f);

		set_view(glm::inverse(m_camera.get_matrix()));
		set_projection(projection);

		float time = input->get_delta_time();
		m_fps = (time != 0.0f ? 1.0f / time : 0.0f);
	}

	virtual void render() override
	{
		if (ImGui::Begin("Stats")) {
			ImGui::Text("FPS: %.2f", m_fps);
			ImGui::End();
		}
	}

	virtual void terminate() override
	{
	}

    private:
	mc::transform m_camera;
	float m_sensitivity = 0.005f;
	float m_speed = 5.0f;

	float m_fps = 0.0;
};

std::unique_ptr<mc::application> create_app()
{
	return std::make_unique<mineclone_application>();
}
