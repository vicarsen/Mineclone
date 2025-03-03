#include "mineclonelib/world/blocks.h"
#include "mineclonelib/io/assets.h"

#include <memory>

namespace mc
{
namespace world
{
face_id face_registry::register_face(std::unique_ptr<face> &&face)
{
	face_id id = m_faces.size();
	m_faces.emplace_back(std::move(face));
	return id;
}

block_id block_registry::register_block(std::unique_ptr<block> &&block)
{
	block_id id = m_blocks.size();
	m_blocks.emplace_back(std::move(block));
	return id;
}

class simple_face : public face {
    public:
	simple_face(const char *texture)
		: m_texture(texture)
	{
	}

	virtual ~simple_face()
	{
	}

	virtual const char *get_texture() const override
	{
		return m_texture.c_str();
	}

    private:
	std::string m_texture;
};

class simple_block : public block {
    public:
	simple_block(face_id face, bool opaque = true)
	{
		for (int i = 0; i < 6; i++) {
			m_faces[i] = face;
		}

		m_opaque = opaque;
	}

	simple_block(face_id east, face_id west, face_id up, face_id down,
		     face_id north, face_id south)
	{
		m_faces[0] = east;
		m_faces[1] = west;
		m_faces[2] = up;
		m_faces[3] = down;
		m_faces[4] = north;
		m_faces[5] = south;
	}
};

namespace faces
{
face_registry *get_registry()
{
	static face_registry reg;
	return &reg;
}

face_id air =
	get_registry()->register_face(std::make_unique<simple_face>(nullptr));
face_id dirt = get_registry()->register_face(std::make_unique<simple_face>(
	ASSET_PATH("mineclone/textures/dirt.png")));
}

namespace blocks
{
block_registry *get_registry()
{
	static block_registry reg;
	return &reg;
}

block_id air = get_registry()->register_block(
	std::make_unique<simple_block>(faces::air, false));
block_id dirt = get_registry()->register_block(
	std::make_unique<simple_block>(faces::dirt));
}
}
}
