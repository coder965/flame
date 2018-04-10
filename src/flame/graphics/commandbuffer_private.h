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
			Graphics *g;
			VkCommandPool v;
		};
	}
}
