#pragma once

#include <functional>
#include <memory>
#include <string>

#include <flame/graphics/graphics.h>

namespace tke
{
	enum WindowStyle
	{
		WindowStyleFrame = 1 << 0,
		WindowStyleResizable = 1 << 1,
		WindowStyleMaximized = 1 << 2,
		WindowStyleFullscreen = 1 << 3
	};

	struct Framebuffer;
	struct Texture;
	class Node;

	class Application
	{
		void *hWnd;
		int window_cx;
		int window_cy;
		int window_style;
		std::string window_title;
		VkSurfaceKHR window_surface;
		VkSwapchainKHR swapchain;
		Texture *window_images[2];
		std::shared_ptr<Framebuffer> window_framebuffers[2];

		Node *root_node;

		Application(int _window_cx, int _window_cy, int _window_style, const std::string &_window_title);

		void create_swapchain();

		void set_window_size(int cx, int cy, int style);
		void set_window_maximized(bool v);

		void add_keydown_listener(const std::function<void(int)> &e);
		void add_keyup_listener(const std::function<void(int)> &e);
		void add_char_listener(const std::function<void(int)> &e);
		void add_resize_listener(const std::function<void(int, int)> &e);
		void add_destroy_listener(const std::function<void()> &e);

		void remove_keydown_listener(const std::function<void(int)> &e);
		void remove_keyup_listener(const std::function<void(int)> &e);
		void remove_char_listener(const std::function<void(int)> &e);
		void remove_resize_listener(const std::function<void(int, int)> &e);
		void remove_destroy_listener(const std::function<void()> &e);

		void add_after_frame_event(const std::function<void()> &e);

		void add_cb(VkCommandBuffer cb);

	};
}
