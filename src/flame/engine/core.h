#pragma once

#include <functional>

#include <flame/global.h>
#include <flame/math/math.h>
#include <flame/graphics/graphics.h>
#include <flame/entity/node.h>

namespace tke
{
	struct Framebuffer;
	struct CommandBuffer;

	enum WindowStyle
	{
		WindowStyleFrame = 1 << 0,
		WindowStyleResize = 1 << 1,
		WindowStyleFullscreen = 1 << 2
	};

	extern void *hWnd;
	extern int window_cx;
	extern int window_cy;
	extern std::shared_ptr<Framebuffer> window_framebuffers[2];
	extern VkSemaphore window_imageAvailable;
	extern uint32_t window_imageIndex;
	extern std::uint32_t window_style;

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

	long long get_now_time_ns();

	extern Node *root_node;

	// must call in main thread
	int init(bool vulkan_debug, const std::string &path, int rcx, int rcy, int _window_cx, int _window_cy, const std::string &title, unsigned int _window_style, bool _only_2d = false);

	std::pair<glm::ivec2, unsigned int> getWin32WndProp();

	// must call in main thread
	void begin_frame();

	void add_to_drawlist(VkCommandBuffer cb);

	// must call in main thread
	void end_frame();

	void run(PF_EVENT0 _on_render);
}