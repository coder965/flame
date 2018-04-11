#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct BufferPrivate;

		struct Buffer
		{
			BufferPrivate *_priv;
		};

		FLAME_GRAPHICS_EXPORTS Buffer *create_buffer(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_buffer(Device *d, Buffer *b);
	}
}

