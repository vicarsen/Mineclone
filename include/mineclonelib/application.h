#pragma once

#include "mineclonelib/io/window.h"
#include "mineclonelib/io/input.h"

#include "mineclonelib/render/context.h"
#include "mineclonelib/render/gui.h"

#include "mineclonelib/render/world.h"
#include <memory>

namespace mc
{
class application {
    public:
	application();
	virtual ~application();

	virtual void init()
	{
	}

	virtual void update()
	{
	}

	virtual void terminate()
	{
	}

	void run();

	void set_view(const glm::mat4 &view);
	void set_projection(const glm::mat4 &projection);

	inline window *get_window() const
	{
		return m_window.get();
	}

	inline input_manager *get_input() const
	{
		return m_input.get();
	}

	inline render::context *get_render_ctx() const
	{
		return m_render_ctx.get();
	}

	inline render::gui_context *get_gui_ctx() const
	{
		return m_gui_ctx.get();
	}

	inline render::world_renderer *get_world_renderer() const
	{
		return m_world_renderer.get();
	}

	static inline application *get() noexcept
	{
		return s_instance;
	}

    private:
	static application *s_instance;

	std::unique_ptr<window> m_window;
	std::unique_ptr<input_manager> m_input;
	std::unique_ptr<render::context> m_render_ctx;
	std::unique_ptr<render::gui_context> m_gui_ctx;

	std::unique_ptr<render::world_renderer> m_world_renderer;

	glm::mat4 m_view;
	glm::mat4 m_projection;
};
}
