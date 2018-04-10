#pragma once

#include "swapchain.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct SwapchainPrivate
		{
			VkSurfaceKHR surface;
			VkFormat format;
			VkSwapchainKHR swapchain;
			VkImage images[2];
			VkImageView image_views[2];
			VkSemaphore image_avalible;
		};
	}
}
