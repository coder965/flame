#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct CommandbufferPrivate;
		struct Renderpass;
		struct Framebuffer;

		struct Commandbuffer
		{
			CommandbufferPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void begin();
			FLAME_GRAPHICS_EXPORTS void begin_renderpass(Renderpass *r, Framebuffer *f);
			FLAME_GRAPHICS_EXPORTS void end_renderpass();
			FLAME_GRAPHICS_EXPORTS void end();
		};

		struct CommandpoolPrivate;

		struct Commandpool
		{
			CommandpoolPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS  Commandbuffer* create_commandbuffer(bool sub = false);
			FLAME_GRAPHICS_EXPORTS  void destroy_commandbuffer(Commandbuffer *c);
		};

		FLAME_GRAPHICS_EXPORTS Commandpool *create_commandpool(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_commandpool(Device *d, Commandpool *p);
	}
}
