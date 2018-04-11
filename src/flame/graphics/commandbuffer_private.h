#include "commandbuffer.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct CommandbufferPrivate
		{
			VkCommandBuffer v;
		};

		struct CommandpoolPrivate
		{
			Device *d;
			VkCommandPool v;
		};
	}
}
