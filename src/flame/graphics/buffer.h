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
		struct Commandbuffer;

		enum BufferUsage
		{
			BufferUsageTransferSrc = 1 << 0,
			BufferUsageTransferDst = 1 << 1,
			BufferUsageUniformBuffer = 1 << 2,
			BufferUsageStorageBuffer = 1 << 3,
			BufferUsageIndexBuffer = 1 << 4,
			BufferUsageVertexBuffer = 1 << 5,
			BufferUsageIndirectBuffer = 1 << 6
		};

		struct BufferPrivate;

		struct Buffer
		{
			int size;

			BufferPrivate *_priv;

#if defined(FLAME_GRAPHICS_VULKAN)
			void *mapped;
			FLAME_GRAPHICS_EXPORTS void map(int offset = 0, int _size = 0);
			FLAME_GRAPHICS_EXPORTS void unmap();
			FLAME_GRAPHICS_EXPORTS void flush();
#else
			FLAME_GRAPHICS_EXPORTS void data(BufferUsage usage, int _size, void *data);
#endif
		};

#if defined(FLAME_GRAPHICS_VULKAN)
		FLAME_GRAPHICS_EXPORTS Buffer *create_buffer(Device *d, int size, int usage, int mem_prop);
#else
		FLAME_GRAPHICS_EXPORTS Buffer *create_buffer(Device *d);
#endif
		FLAME_GRAPHICS_EXPORTS void destroy_buffer(Device *d, Buffer *b);
	}
}

