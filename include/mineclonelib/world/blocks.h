#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace mc
{
namespace world
{
using block_id = uint8_t;
using face_id = uint16_t;

class face {
    public:
	face() = default;
	virtual ~face() = default;

	virtual const char *get_texture() const = 0;
};

class face_registry {
    public:
	face_registry() = default;
	~face_registry() = default;

	face_id register_face(std::unique_ptr<face> &&face);

	inline face *get(face_id face) const noexcept
	{
		return m_faces[face].get();
	}

    private:
	std::vector<std::unique_ptr<face> > m_faces;
};

enum class block_face {
	east, // x+
	west, // x-
	up, // y+
	down, // y-
	north, // z+
	south // z-
};

class block {
    public:
	block() = default;
	virtual ~block() = default;

	inline face_id get_face(block_face face) const noexcept
	{
		return m_faces[static_cast<int>(face)];
	}

    protected:
	face_id m_faces[6];
};

class block_registry {
    public:
	block_registry() = default;
	~block_registry() = default;

	block_id register_block(std::unique_ptr<block> &&block);

	inline block *get(block_id block) const noexcept
	{
		return m_blocks[block].get();
	}

    private:
	std::vector<std::unique_ptr<block> > m_blocks;
};

namespace faces
{
face_registry *get_registry();

extern face_id air;
extern face_id dirt;
}

namespace blocks
{
block_registry *get_registry();

extern block_id air;
extern block_id dirt;
}
}
}
