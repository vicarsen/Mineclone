#pragma once

#include "mineclonelib/render/gui.h"

namespace mc
{
namespace render
{
class gl_gui_context : public gui_context {
    public:
	gl_gui_context(context *ctx);
	virtual ~gl_gui_context();

	virtual void begin() override;
	virtual void present() override;
};
}
}
