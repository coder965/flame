#pragma once

#include <functional>

#include "global.h"
#include "math/math.h"
#include "graphics/graphics.h"
#include "entity/node.h"

namespace tke
{
	struct UniformBuffer;
	struct Image;
	struct RenderPass;
	struct Framebuffer;
	struct CommandBuffer;

	extern UniformBuffer *constantBuffer;

	void processCmdLine(const std::string &str, bool record = true);

	enum WindowStyle
	{
		WindowStyleFrame = 1 << 0,
		WindowStyleResize = 1 << 1,
		WindowStyleFullscreen = 1 << 2
	};

	extern void *hWnd;
	extern int window_cx;
	extern int window_cy;
	extern VkSurfaceKHR window_surface;
	extern VkSwapchainKHR swapchain;
	extern Image *window_images[2];
	extern std::shared_ptr<Framebuffer> window_framebuffers[2];
	extern VkSemaphore window_imageAvailable;
	extern uint32_t window_imageIndex;
	extern VkFence frameDone;
	extern PF_EVENT1 onKeyDown;
	extern PF_EVENT1 onKeyUp;
	extern PF_EVENT1 onChar;
	extern PF_EVENT2 onMouseLeftDown;
	extern PF_EVENT2 onMouseLeftUp;
	extern PF_EVENT2 onMouseMiddleDown;
	extern PF_EVENT2 onMouseMiddleUp;
	extern PF_EVENT2 onMouseRightDown;
	extern PF_EVENT2 onMouseRightUp;
	extern PF_EVENT2 onMouseMove;
	extern PF_EVENT1 onMouseWheel;
	extern PF_EVENT0 onRender;
	extern PF_EVENT0 onDestroy;
	extern std::uint32_t window_style;

	long long get_now_time_ms();

	extern Node *root_node;

	// must call in main thread
	int init(bool vulkan_debug, const std::string &path, int rcx, int rcy, int _window_cx, int _window_cy, const std::string &title, unsigned int _window_style, bool _only_2d = false);

	std::pair<glm::ivec2, unsigned int> getWin32WndProp();

	enum EventType
	{
		EventTypeEvent,
		EventTypeOnlyOne // only one can exist in the list
	};
	void add_before_frame_event(const std::function<void()>&, int id = -1, EventType event_type = EventTypeEvent);

	// must call in main thread
	void begin_frame();

	void add_to_drawlist(VkCommandBuffer cb);

	// must call in main thread
	void end_frame();

	void run();
}
