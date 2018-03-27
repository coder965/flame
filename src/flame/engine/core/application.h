#pragma once

#include <memory>
#include <string>
#include <vector>
#include <list>

#include <flame/global.h>
#include <flame/engine/graphics/graphics.h>

namespace flame
{
	void add_after_frame_event(const std::function<void()> &e);

	struct Framebuffer;
	struct Texture;
	struct TextureView;
	class Node;

	class Application
	{
	public:
		Node *root_node;
		VkSemaphore render_finished;
	private:
		std::vector<VkCommandBuffer> cbs;
	public:
		Application(int _window_cx, int _window_cy, int _window_style, const std::string &_window_title);
		~Application();
	protected:
		virtual void on_render() = 0;
	public:
		void update();

		void create_swapchain();

		void set_window_size(int cx, int cy, int style);
		void set_window_maximized(bool v);

		void add_cb(VkCommandBuffer cb);

	};

	extern Application *app;
}
