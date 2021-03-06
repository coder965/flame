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

#include "vao.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
#if !defined(FLAME_GRAPHICS_VULKAN)
		inline void vertex_attribute_data(VertexAttributeType t, GLuint &out_type, int &out_count)
		{
			switch (t)
			{
			case VertexAttributeFloat:
				out_type = GL_FLOAT;
				out_count = 1;
				break;
			case VertexAttributeFloat2:
				out_type = GL_FLOAT;
				out_count = 2;
				break;
			case VertexAttributeFloat3:
				out_type = GL_FLOAT;
				out_count = 3;
				break;
			case VertexAttributeFloat4:
				out_type = GL_FLOAT;
				out_count = 4;
				break;
			case VertexAttributeByte4:
				out_type = GL_UNSIGNED_BYTE;
				out_count = 4;
				break;
			default:
				out_type = 0;
				out_count = 0;
			}
		}

		struct VaoPrivate
		{
			GLuint v;
		};
#endif
	}
}

