#pragma once

#include "mineclonelib/log.h"

DECLARE_LOG_CATEGORY(Render);

namespace mc
{
namespace render
{
enum class render_api { none = 0, opengl, vulkan };
}
}
