#pragma once

#include "mineclonelib/render/world.h"

#include <glad/gl.h>

#include <stack>

namespace mc
{
namespace render
{
class gl_world_renderer : public world_renderer {
    public:
	gl_world_renderer(context *ctx);
	virtual ~gl_world_renderer();

	virtual chunk_handle alloc_chunk() override;
	virtual void free_chunk(chunk_handle handle) override;

	virtual void upload_chunk(chunk_handle handle,
				  mc::world::chunk_draw_data *draw_data,
				  const glm::mat4 &model) override;

	virtual void render(const glm::mat4 &view,
			    const glm::mat4 &projection) override;

    private:
	void grow(uint32_t capacity);

    private:
	std::stack<uint32_t> m_free;
	GLuint m_chunks;
	GLuint m_uniforms;
	GLuint m_indirect;
	uint32_t m_capacity;

	GLuint m_texarray;

	GLuint m_program;
	GLint m_view;
	GLint m_projection;
	GLint m_texture;
};
}
}
