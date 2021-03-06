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

#include "vao_private.h"
#include "buffer_private.h"

namespace flame
{
	namespace graphics
	{
		void Vao::bind_buffer(Buffer *b)
		{
			glBindVertexArray(_priv->v);
			glBindBuffer(GL_ARRAY_BUFFER, b->_priv->v);
		}

		void Vao::vertex_attribute_pointer(int index, VertexAttributeType vtx_attrib,
			bool normalized, int stride, int offset)
		{
			glBindVertexArray(_priv->v);
			glEnableVertexAttribArray(index);
			GLuint attrib_type;
			int attrib_count;
			vertex_attribute_data(vtx_attrib, attrib_type, attrib_count);
			glVertexAttribPointer(index, attrib_count, attrib_type, normalized,
				stride, (void*)offset);
		}

#if !defined(FLAME_GRAPHICS_VULKAN)
		Vao *create_vao()
		{
			auto v = new Vao;

			v->_priv = new VaoPrivate;

			glGenVertexArrays(1, &v->_priv->v);

			return v;
		}

		void destroy_vao(Vao *v)
		{
			glDeleteVertexArrays(1, &v->_priv->v);

			delete v->_priv;
			delete v;
		}
#endif
	}
}

