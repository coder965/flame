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
		struct Pipeline;
		struct Descriptorset;
		struct Buffer;

		struct Commandbuffer
		{
			CommandbufferPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void begin(bool once = false);

			FLAME_GRAPHICS_EXPORTS void begin_renderpass(Renderpass *r, Framebuffer *f);
			FLAME_GRAPHICS_EXPORTS void end_renderpass();
			FLAME_GRAPHICS_EXPORTS void bind_pipeline(Pipeline *p);
			FLAME_GRAPHICS_EXPORTS void bind_descriptorset(Descriptorset *s);
			FLAME_GRAPHICS_EXPORTS void bind_vertexbuffer(Buffer *b);
			FLAME_GRAPHICS_EXPORTS void bind_indexbuffer(Buffer *b, IndiceType t);
			FLAME_GRAPHICS_EXPORTS void draw(int count);
			FLAME_GRAPHICS_EXPORTS void draw_indexed(int count, int first_index);

			FLAME_GRAPHICS_EXPORTS void copy_buffer(Buffer *src, Buffer *dst, int src_offset, int dst_offset, int size);

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
