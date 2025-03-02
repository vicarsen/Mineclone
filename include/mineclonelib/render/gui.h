#pragma once

#include "mineclonelib/render/context.h"

namespace mc
{
namespace render
{
class gui_context {
    public:
	gui_context(context *ctx);
	virtual ~gui_context();

	virtual void begin();
	virtual void present();

	static std::unique_ptr<gui_context> create(context *ctx);

    protected:
	context *m_ctx;
};
}
}
