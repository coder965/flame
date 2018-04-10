#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct CommandbufferPrivate;

		struct Commandbuffer
		{
			CommandbufferPrivate *_priv;
		};

		struct CommandpoolPrivate;

		struct Commandpool
		{
			CommandpoolPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS  Commandbuffer* create_commandbuffer(bool sub = false);
			FLAME_GRAPHICS_EXPORTS  void destroy_commandbuffer(Commandbuffer *c);
		};

		FLAME_GRAPHICS_EXPORTS Commandpool *create_commandpool(Graphics *g);
		FLAME_GRAPHICS_EXPORTS void destroy_commandpool(Graphics *g, Commandpool *q);
	}
}
