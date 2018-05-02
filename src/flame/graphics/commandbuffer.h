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
		struct Texture;

		struct BufferCopy
		{
			int src_offset;
			int dst_offset;
			int size;
		};

		struct BufferImageCopy
		{
			int buffer_offset;
			int image_width;
			int image_height;
			int image_level;
		};

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
			FLAME_GRAPHICS_EXPORTS void push_constant(int shader_stage, int offset, int size, void *data);
			FLAME_GRAPHICS_EXPORTS void draw(int count, int instance_count, int first_instance);
			FLAME_GRAPHICS_EXPORTS void draw_indexed(int count, int first_index, int instance_count, int first_instance);
			FLAME_GRAPHICS_EXPORTS void dispatch(int x, int y, int z);

			FLAME_GRAPHICS_EXPORTS void copy_buffer(Buffer *src, Buffer *dst, int copy_count, BufferCopy *copies);
			FLAME_GRAPHICS_EXPORTS void change_texture_layout(Texture *t, TextureLayout from, TextureLayout to,
				int base_level = 0, int level_count = 0, int base_layer = 0, int layer_count = 0);
			FLAME_GRAPHICS_EXPORTS void copy_buffer_to_image(Buffer *src, Texture *dst, int copy_count, BufferImageCopy *copies);

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
