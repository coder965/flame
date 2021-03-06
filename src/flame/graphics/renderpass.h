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

#include <vector>

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct RenderpassPrivate;

		struct Renderpass
		{
			bool valid;

			RenderpassPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void add_attachment(Format format, bool clear);
			FLAME_GRAPHICS_EXPORTS void add_subpass(const std::initializer_list<int> &color_attachments, int depth_attachment);
			FLAME_GRAPHICS_EXPORTS void add_dependency(int src_subpass, int dst_subpass);
			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Renderpass *create_renderpass(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_renderpass(Device *d, Renderpass *r);
	}
}

