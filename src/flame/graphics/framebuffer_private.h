#pragma once

#include <vector>

#include "framebuffer.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct Renderpass;

		struct FramebufferPrivate
		{
			Graphics *g;
			Renderpass *renderpass;
			std::vector<VkImageView> views;
			VkFramebuffer v;
		};
	}
}
