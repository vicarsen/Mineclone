#pragma once

#define CHUNK_SIZE_LOG 6
#define CHUNK_SIZE 64
#define CHUNK_PADDING 1
#define CHUNK_BEGIN (CHUNK_PADDING)
#define CHUNK_END (CHUNK_SIZE + CHUNK_PADDING)
#define CHUNK_TOTAL (CHUNK_SIZE + 2 * CHUNK_PADDING)

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

struct chunk {
	block_id blocks[CHUNK_TOTAL][CHUNK_TOTAL][CHUNK_TOTAL] = {};

	inline block_id get(int x, int y, int z) const noexcept
	{
		return blocks[x][y][z];
	}

	inline void set(int x, int y, int z, block_id block) noexcept
	{
		blocks[x][y][z] = block;
	}
};

class chunk_draw_data_generator {
    public:
	virtual ~chunk_draw_data_generator() = default;

	virtual chunk_draw_data generate(chunk *ch) const = 0;
};

class simple_chunk_draw_data_generator final
	: public chunk_draw_data_generator {
    public:
	virtual ~simple_chunk_draw_data_generator() = default;

	virtual chunk_draw_data generate(chunk *ch) const override;
};
}
}
