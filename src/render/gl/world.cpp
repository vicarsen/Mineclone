#include "mineclonelib/render/gl/world.h"
#include "mineclonelib/render/gl/utils.h"
#include "mineclonelib/render/world.h"
#include "mineclonelib/world/blocks.h"
#include "mineclonelib/world/chunk.h"
#include "mineclonelib/io/assets.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#define CHUNK_MESH_SIZE (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 3)

namespace mc
{
namespace render
{
struct packed_face {
	uint32_t geometry;
	uint32_t shading;
};

static packed_face pack_face(const mc::world::face_draw_data &face)
{
	uint32_t geometry =
		(static_cast<uint32_t>(face.x)) |
		(static_cast<uint32_t>(face.y) << (CHUNK_SIZE_LOG)) |
		(static_cast<uint32_t>(face.z) << (2 * CHUNK_SIZE_LOG)) |
		(static_cast<uint32_t>(face.ao) << (3 * CHUNK_SIZE_LOG));

	uint32_t shading = face.face;

	return { geometry, shading };
}

struct chunk_uniform_data {
	glm::mat4 model;
	uint32_t normal;
	uint32_t padding[3];
};

struct indirect_draw_command {
	unsigned int count;
	unsigned int instance_count;
	unsigned int first_vertex;
	unsigned int base_instance;
};

static void gl_write_zeros(GLuint buffer, uint32_t offset, uint32_t size)
{
	std::vector<uint8_t> zeros(size, 0);
	glNamedBufferSubData(buffer, offset, size, zeros.data());
}

static GLuint gl_grow_buffer(GLuint buffer, uint32_t prev, uint32_t next)
{
	GLuint new_buffer;
	glCreateBuffers(1, &new_buffer);
	glNamedBufferStorage(new_buffer, next, NULL, GL_DYNAMIC_STORAGE_BIT);

	if (buffer != 0) {
		glCopyNamedBufferSubData(buffer, new_buffer, 0, 0, prev);
		glDeleteBuffers(1, &buffer);
	}

	return new_buffer;
}

gl_world_renderer::gl_world_renderer(context *ctx)
	: world_renderer(ctx)
	, m_chunks(0)
	, m_uniforms(0)
	, m_indirect(0)
	, m_capacity(0)
{
	grow(9);

	GLuint vertex_shader =
		gl::load_shader(ASSET_PATH("mineclone/shaders/gl/chunk.vert"),
				GL_VERTEX_SHADER);

	GLuint fragment_shader =
		gl::load_shader(ASSET_PATH("mineclone/shaders/gl/chunk.frag"),
				GL_FRAGMENT_SHADER);

	world::face_registry *freg = world::faces::get_registry();

	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_texarray);

	glTextureParameteri(m_texarray, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texarray, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texarray, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_texarray, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureStorage3D(m_texarray, 1, GL_RGBA8, 32, 32, freg->size());
	for (uint32_t i = 0; i < freg->size(); i++) {
		world::face *face = freg->get(i);
		const char *path = face->get_texture();
		if (path == nullptr) {
			continue;
		}

		stbi_set_flip_vertically_on_load(true);

		int width, height, channels;
		unsigned char *data =
			stbi_load(path, &width, &height, &channels, 0);

		glTextureSubImage3D(m_texarray, 0, 0, 0, i, width, height, 1,
				    channels == 3 ? GL_RGB : GL_RGBA,
				    GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	m_program = glCreateProgram();

	glAttachShader(m_program, vertex_shader);
	glAttachShader(m_program, fragment_shader);

	glLinkProgram(m_program);

	glDetachShader(m_program, vertex_shader);
	glDetachShader(m_program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	if (!gl::check_link_status(m_program)) {
		glDeleteProgram(m_program);
		m_program = 0;
	}

	m_view = glGetUniformLocation(m_program, "u_view");
	m_projection = glGetUniformLocation(m_program, "u_projection");
	m_texture = glGetUniformLocation(m_program, "u_texture");
}

gl_world_renderer::~gl_world_renderer()
{
	if (m_program) {
		glDeleteProgram(m_program);
	}

	if (m_texarray) {
		glDeleteTextures(1, &m_texarray);
	}

	if (m_chunks) {
		glDeleteBuffers(1, &m_chunks);
	}

	if (m_uniforms) {
		glDeleteBuffers(1, &m_uniforms);
	}

	if (m_indirect) {
		glDeleteBuffers(1, &m_indirect);
	}
}

chunk_handle gl_world_renderer::alloc_chunk()
{
	if (m_free.empty()) {
		grow(m_capacity * 1.5);
	}

	chunk_handle handle = m_free.top();
	m_free.pop();

	return handle;
}

void gl_world_renderer::free_chunk(chunk_handle handle)
{
	gl_write_zeros(m_indirect,
		       (handle - 1) * 6 * sizeof(indirect_draw_command),
		       6 * sizeof(indirect_draw_command));
	m_free.push(handle);
}

void gl_world_renderer::upload_chunk(chunk_handle handle,
				     mc::world::chunk_draw_data *draw_data,
				     const glm::mat4 &model)
{
	std::vector<packed_face> faces[6];

	for (uint32_t i = 0; i < draw_data->faces.size(); i++) {
		world::block_face normal = draw_data->faces[i].normal;
		packed_face face = pack_face(draw_data->faces[i]);
		faces[static_cast<uint32_t>(normal)].push_back(face);
	}

	indirect_draw_command cmd = { .count = 0,
				      .instance_count = 0,
				      .first_vertex = 0,
				      .base_instance = 0 };

	chunk_uniform_data uniforms;
	uniforms.model = model;

	uint32_t fidx = (handle - 1) * CHUNK_MESH_SIZE;
	uint32_t idx = (handle - 1) * 6;

	for (uint32_t i = 0; i < 6; i++) {
		cmd.count = faces[i].size() * 6;
		cmd.instance_count = 1;
		cmd.first_vertex = fidx * 6;

		uniforms.normal = i;

		glNamedBufferSubData(m_chunks, fidx * sizeof(packed_face),
				     faces[i].size() * sizeof(packed_face),
				     faces[i].data());

		glNamedBufferSubData(m_indirect,
				     idx * sizeof(indirect_draw_command),
				     sizeof(indirect_draw_command), &cmd);

		glNamedBufferSubData(m_uniforms,
				     idx * sizeof(chunk_uniform_data),
				     sizeof(chunk_uniform_data), &uniforms);

		fidx += faces[i].size();
		idx++;
	}
}

void gl_world_renderer::render(const glm::mat4 &view,
			       const glm::mat4 &projection)
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_chunks);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_uniforms);

	glBindTextureUnit(1, m_texarray);

	glUseProgram(m_program);

	glUniformMatrix4fv(m_view, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(m_projection, 1, GL_FALSE,
			   glm::value_ptr(projection));
	glUniform1i(m_texture, 1);

	glMultiDrawArraysIndirect(GL_TRIANGLES, NULL, m_capacity * 6, 0);
}

void gl_world_renderer::grow(uint32_t capacity)
{
	m_chunks = gl_grow_buffer(
		m_chunks, m_capacity * CHUNK_MESH_SIZE * sizeof(packed_face),
		capacity * CHUNK_MESH_SIZE * sizeof(packed_face));

	m_uniforms = gl_grow_buffer(m_uniforms,
				    m_capacity * 6 * sizeof(chunk_uniform_data),
				    capacity * 6 * sizeof(chunk_uniform_data));

	m_indirect = gl_grow_buffer(
		m_indirect, m_capacity * 6 * sizeof(indirect_draw_command),
		capacity * 6 * sizeof(indirect_draw_command));

	gl_write_zeros(
		m_indirect, m_capacity * 6 * sizeof(indirect_draw_command),
		(capacity - m_capacity) * 6 * sizeof(indirect_draw_command));

	for (uint32_t i = capacity; i != m_capacity; i--) {
		m_free.emplace(i);
	}

	m_capacity = capacity;
}
}
}
