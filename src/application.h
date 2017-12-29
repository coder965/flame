#pragma once

#include <functional>

#include "refl.h"
#include "math/math.h"
#include "event.h"
#include "graphics/graphics.h"

namespace tke
{
	struct UniformBuffer;
	struct Image;
	struct RenderPass;
	struct Framebuffer;
	struct CommandBuffer;

	IMPL(nullptr) UniformBuffer *constantBuffer;

	void processCmdLine(const std::string &str, bool record = true);

	enum WindowStyle
	{
		WindowStyleFrame = 1 << 0,
		WindowStyleResize = 1 << 1,
		WindowStyleFullscreen = 1 << 2
	};

	// must call in main thread
	int init(bool vulkan_debug, const std::string &path, int rcx, int rcy, int _window_cx, int _window_cy, const std::string &title, unsigned int _window_style, bool _only_2d = false);

	IMPL() void *hWnd;
	IMPL() int window_cx;
	IMPL() int window_cy;
	IMPL() VkSurfaceKHR window_surface;
	IMPL() VkSwapchainKHR swapchain;
	IMPL() Image *window_images[2];
	IMPL() std::shared_ptr<Framebuffer> window_framebuffers[2];
	IMPL() VkSemaphore window_imageAvailable;
	IMPL() uint32_t window_imageIndex;
	IMPL() VkFence frameDone;
	IMPL() uint32_t FPS;
	IMPL(nullptr) PF_EVENT1 onKeyDown;
	IMPL(nullptr) PF_EVENT1 onKeyUp;
	IMPL(nullptr) PF_EVENT1 onChar;
	IMPL(nullptr) PF_EVENT2 onMouseLeftDown;
	IMPL(nullptr) PF_EVENT2 onMouseLeftUp;
	IMPL(nullptr) PF_EVENT2 onMouseMiddleDown;
	IMPL(nullptr) PF_EVENT2 onMouseMiddleUp;
	IMPL(nullptr) PF_EVENT2 onMouseRightDown;
	IMPL(nullptr) PF_EVENT2 onMouseRightUp;
	IMPL(nullptr) PF_EVENT2 onMouseMove;
	IMPL(nullptr) PF_EVENT1 onMouseWheel;
	IMPL(nullptr) PF_EVENT0 onRender;
	IMPL(nullptr) PF_EVENT0 onDestroy;
	IMPL() std::uint32_t window_style;

	std::pair<glm::ivec2, unsigned int> getWin32WndProp();

	enum EventType
	{
		EventTypeEvent,
		EventTypeOnlyOne
	};
	void addBeforeFrameEvent(const std::function<void()>&, int id = -1, EventType event_type = EventTypeEvent);

	// must call in main thread
	void beginFrame(bool clearBackground);

	void addCb(VkCommandBuffer cb);

	// must call in main thread
	void endFrame();

	void run();
}
