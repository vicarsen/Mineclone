#pragma once

#include "mineclonelib/world/chunk.h"

#include <glm/glm.hpp>

#include <vector>

namespace mc
{
namespace world
{
struct world_state {
	std::vector<world::chunk> chunks;
	std::vector<glm::ivec3> coords;
};
}
}
