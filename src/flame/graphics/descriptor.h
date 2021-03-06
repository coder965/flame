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
		struct Pipeline;
		struct Buffer;
		struct Texture;
		struct Textureview;
		struct Sampler;

		struct DescriptorsetlayoutPrivate;

		struct Descriptorsetlayout
		{
			DescriptorsetlayoutPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void clear_bindings();
			FLAME_GRAPHICS_EXPORTS void add_binding(ShaderResourceType type, int binding, int count, ShaderType shader_type);
			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Descriptorsetlayout *create_descriptorsetlayout(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_descriptorsetlayout(Device *d, Descriptorsetlayout *l);

		struct DescriptorsetPrivate;

		struct Descriptorset
		{
			DescriptorsetPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void set_uniformbuffer(int binding, int index, Buffer *b, int offset = 0, int range = 0);
			FLAME_GRAPHICS_EXPORTS void set_storagebuffer(int binding, int index, Buffer *b, int offset = 0, int range = 0);
			FLAME_GRAPHICS_EXPORTS void set_texture(int binding, int index, Textureview *v, Sampler *sampler);
			FLAME_GRAPHICS_EXPORTS void set_storagetexture(int binding, int index, Textureview *v);
		};

		struct DescriptorpoolPrivate;

		struct Descriptorpool
		{
			DescriptorpoolPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS Descriptorset *create_descriptorset(Descriptorsetlayout *l);
			FLAME_GRAPHICS_EXPORTS Descriptorset *create_descriptorset(Pipeline *p, int index);
			FLAME_GRAPHICS_EXPORTS void destroy_descriptorset(Descriptorset *s);
		};

		FLAME_GRAPHICS_EXPORTS Descriptorpool *create_descriptorpool(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_descriptorpool(Device *d, Descriptorpool *p);
	}
}

