#pragma once

#define CHUNK_SIZE 32
#define CHUNK_PADDING 1
#define CHUNK_BEGIN (CHUNK_SIZE + CHUNK_PADDING)
#define CHUNK_END (CHUNK_SIZE + 2 * CHUNK_PADDING)

#include "mineclonelib/world/blocks.h"

namespace mc
{
namespace world
{
struct face_draw_data {
	face_id face;
	block_face normal;
	uint8_t ao;
	uint8_t x, y, z;
};

struct chunk_draw_data {
	std::vector<face_draw_data> faces;
};

class chunk {
    public:
	chunk();
	virtual ~chunk();

	inline block_id get(int x, int y, int z) const noexcept
	{
		return m_blocks[x][y][z];
	}

	inline void set(int x, int y, int z, block_id block) noexcept
	{
		m_blocks[x][y][z] = block;
	}

    private:
	block_id m_blocks[CHUNK_END][CHUNK_END][CHUNK_END];
};

class chunk_draw_data_generator {
    public:
	virtual chunk_draw_data generate(chunk *ch) = 0;
};

class simple_chunk_draw_data_generator : public chunk_draw_data_generator {
    public:
	virtual chunk_draw_data generate(chunk *ch) override;
};
}
}
