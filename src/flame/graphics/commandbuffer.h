//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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
			FLAME_GRAPHICS_EXPORTS void set_viewport(const Ivec2 &pos, const Ivec2 &size);
			FLAME_GRAPHICS_EXPORTS void set_scissor(const Ivec2 &pos, const Ivec2 &size);
			FLAME_GRAPHICS_EXPORTS void bind_pipeline(Pipeline *p);
			FLAME_GRAPHICS_EXPORTS void bind_descriptorset(Descriptorset *s);
			FLAME_GRAPHICS_EXPORTS void bind_vertexbuffer(Buffer *b);
			FLAME_GRAPHICS_EXPORTS void bind_indexbuffer(Buffer *b, IndiceType t);
			FLAME_GRAPHICS_EXPORTS void push_constant(int shader_stage, int offset, int size, void *data);
			FLAME_GRAPHICS_EXPORTS void draw(int count, int instance_count, int first_instance);
			FLAME_GRAPHICS_EXPORTS void draw_indexed(int count, int first_index, int vertex_offset, int instance_count, int first_instance);
			FLAME_GRAPHICS_EXPORTS void dispatch(const Ivec3 &v);

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
