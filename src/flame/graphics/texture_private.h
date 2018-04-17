#include "texture.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct TexturePrivate
		{
			Device *d;
			VkImage v;
			VkDeviceMemory m;
		};

		struct TextureviewPrivate
		{
			VkImageView v;
		};
	}
}

