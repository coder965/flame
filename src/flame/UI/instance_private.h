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

#include "instance.h"
#include "UI_private.h"

namespace flame
{
	namespace graphics
	{
		struct Shader;
		struct Pipeline;
		struct Descriptorset;
		struct Texture;
		struct Textureview;
		struct Sampler;
		struct Buffer;
	}

	namespace UI
	{
		struct InstancePrivate
		{
			graphics::Device *d;
			graphics::Shader *vert;
			graphics::Shader *frag;
			graphics::Pipeline *pl;
			graphics::Descriptorset *ds;
			graphics::Texture *font_tex;
			graphics::Textureview *font_view;
			graphics::Sampler *font_sam;
			graphics::Buffer *vtx_buffer;
			graphics::Buffer *idx_buffer;
			Surface *s;
			void *cursors[ImGuiMouseCursor_COUNT];
		};
	}
}

