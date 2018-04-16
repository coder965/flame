#include "commandbuffer.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct CommandbufferPrivate
		{
			Pipeline *current_pipeline;
			VkCommandBuffer v;
		};

		struct CommandpoolPrivate
		{
			Device *d;
			VkCommandPool v;
		};
	}
}
