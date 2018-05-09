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
			void *mapped;

			BufferPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void map(int offset = 0, int _size = 0);
			FLAME_GRAPHICS_EXPORTS void unmap();
			FLAME_GRAPHICS_EXPORTS void flush();
		};

		FLAME_GRAPHICS_EXPORTS Buffer *create_buffer(Device *d, int size, int usage, int mem_prop);
		FLAME_GRAPHICS_EXPORTS void destroy_buffer(Device *d, Buffer *b);
	}
}

