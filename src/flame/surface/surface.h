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

		VkImage images[2];
		std::unique_ptr<TextureView> image_views[2];
		uint image_index;
		VkSemaphore image_available;

		Surface(int _cx, int _cy, int _style, const std::string &_title);
		void set_window_size(int _cx, int _cy, int _style);
		void set_window_maximized(bool v);
		void create_swapchain();
		void acquire_image();
		void present(VkSemaphore wait_semaphore);
	};

	extern Surface *surface;

	void add_resize_listener(const std::function<void(int, int)> &e);
	void remove_resize_listener(const std::function<void(int, int)> &e);
}
