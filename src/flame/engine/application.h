#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <list>

#include <flame/global.h>
#include <flame/graphics/graphics.h>

namespace flame
{
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

	enum WindowStyle
	{
		WindowStyleFrame = 1 << 0,
		WindowStyleResizable = 1 << 1,
		WindowStyleMaximized = 1 << 2,
		WindowStyleFullscreen = 1 << 3
	};

	struct InputState
	{
		bool just_down = false;
		bool just_up = false;
		bool pressing = false;

		InputState();
		void reset();
		void on_down();
		void on_up();
	};

	struct Framebuffer;
	struct Texture;
	class Node;

	class Application
	{
	private:
		void *hWnd;
	public:
		int window_cx;
		int window_cy;
		int window_style;
		std::string window_title;
	private:
		VkSurfaceKHR window_surface;
		VkSwapchainKHR swapchain;
		std::unique_ptr<Texture> window_images[2];
		std::shared_ptr<Framebuffer> window_framebuffers[2];
		uint window_image_index;
		VkSemaphore image_available;
		VkFence frame_finished;
	public:
		Node *root_node;

		int mouseX;
		int mouseY;
		int mousePrevX;
		int mousePrevY;
		int mouseDispX;
		int mouseDispY;
		int mouseScroll;

		InputState mouse_button[3]; // left, right, middle
		InputState key_states[256];
	private:
		std::vector<VkCommandBuffer> cbs;
	public:
		Application(int _window_cx, int _window_cy, int _window_style, const std::string &_window_title);
		~Application();

		void on_lbutton_down(int x, int y);
		void on_lbutton_up(int x, int y);
		void on_mbutton_down(int x, int y);
		void on_mbutton_up(int x, int y);
		void on_rbutton_down(int x, int y);
		void on_rbutton_up(int x, int y);
		void on_mouse_move(int x, int y);
		void on_mouse_wheel(int v);
		void on_key_down(int v);
		void on_key_up(int v);
		void on_char(int v);
		void on_size(int x, int y);
	protected:
		virtual void on_render() = 0;
	public:
		void update();

		Framebuffer *get_curr_framebuffer() const;

		void create_swapchain();

		void set_window_size(int cx, int cy, int style);
		void set_window_maximized(bool v);

		void add_cb(VkCommandBuffer cb);

	};

	extern Application *app;
}
