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

#include <flame/string.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct ShaderResource
		{
			ShaderResourceType type;
			int binding;
			int count;
			std::string name;
		};

		struct ShaderPrivate;

		struct Shader
		{
			MediumString filename;
			std::vector<ShortString> defines;
			ShaderType type;

			ShaderPrivate *_priv;

			inline void add_define(const char *d)
			{
				defines.resize(defines.size() + 1);
				strcpy(defines.back().data, d);
			}

			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Shader *create_shader(Device *d, const char *filename);
		FLAME_GRAPHICS_EXPORTS void destroy_shader(Device *d, Shader *s);
	}
}
