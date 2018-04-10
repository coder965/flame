#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct SwapchainPrivate;

		struct Swapchain
		{
			int cx;
			int cy;

			SwapchainPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS int acquire_image();
		};

		FLAME_GRAPHICS_EXPORTS Swapchain *create_swapchain(Graphics *g, void *, int cx, int cy);
		FLAME_GRAPHICS_EXPORTS void destroy_swapchain(Graphics *g, Swapchain *s);
	}
}

