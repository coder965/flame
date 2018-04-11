#pragma once

#include "swapchain.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct SwapchainPrivate
		{
			Device *d;
			VkSurfaceKHR surface;
			VkFormat format;
			VkSwapchainKHR swapchain;
			VkImage images[2];
			VkImageView image_views[2];
		};
	}
}
