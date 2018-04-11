#include "buffer.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct BufferPrivate
		{
			Device *d;
			VkBuffer v;
		};
	}
}

