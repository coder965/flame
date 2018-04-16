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
			Device *d;
			Renderpass *renderpass;
			int cx;
			int cy;
			std::vector<VkImageView> views;
			VkFramebuffer v;
		};
	}
}
