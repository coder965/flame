#pragma once

#include <memory>

#include <flame/engine/graphics/graphics.h>

namespace flame
{
	struct TextureView;

	enum SurfaceStyle
	{
		SurfaceStyleFrame = 1 << 0,
		SurfaceStyleResizable = 1 << 1,
		SurfaceStyleMaximized = 1 << 2,
		SurfaceStyleFullscreen = 1 << 3
	};

	struct Surface
	{
		int cx;
		int cy;
		int style;
		std::string title;

		VkSurfaceKHR v;
		VkSwapchainKHR swapchain;
		VkImage images[2];
		std::unique_ptr<TextureView> image_views[2];
		uint image_index;
		VkSemaphore image_available;

		Surface(int _window_cx, int _window_cy, int _window_style, const std::string &_window_title);
	};
}
