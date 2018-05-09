#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Semaphore;

		struct SwapchainPrivate;

		struct Swapchain
		{
			int cx;
			int cy;
			Format format;

			SwapchainPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS int acquire_image(Semaphore *signal_semaphore);
		};

		FLAME_GRAPHICS_EXPORTS Swapchain *create_swapchain(Device *d, void *, int cx, int cy);
		FLAME_GRAPHICS_EXPORTS void destroy_swapchain(Device *d, Swapchain *s);
	}
}

