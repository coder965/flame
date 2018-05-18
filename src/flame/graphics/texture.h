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

#include <string>

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Commandpool;
		struct Commandbuffer;
		struct Queue;

		struct TexturePrivate;

		struct Texture
		{
			int cx;
			int cy;
			int level;
			int layer;
			Format format;

			TexturePrivate *_priv;
		};

		struct TextureviewPrivate;

		struct Textureview
		{
			TextureviewPrivate *_priv;
		};

		FLAME_GRAPHICS_EXPORTS Texture *create_texture(Device *d, int cx, int cy, int level, int layer, Format format, int usage, int mem_prop);
		FLAME_GRAPHICS_EXPORTS Texture *create_texture_from_file(Device *d, const std::string &filename, int usage = 0, int mem_prop = 0);
		FLAME_GRAPHICS_EXPORTS void destroy_texture(Device *d, Texture *t);

		FLAME_GRAPHICS_EXPORTS Textureview *create_textureview(Device *d, Texture *t,
			TextureViewType view_type = TextureViewType2D, int base_level = 0, int level_count = 1, int base_layer = 0, int layer_count = 1);
		FLAME_GRAPHICS_EXPORTS void destroy_textureview(Device *d, Textureview *v);
	}
}

