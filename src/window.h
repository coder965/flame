#pragma once

#include "utils.h"
#include "render/vulkan.h"

namespace tke
{
	struct RenderPass;
	extern RenderPass *renderPass_window;
	extern RenderPass *renderPass_window_clear;

	enum WindowStyle
	{
		WindowStyleNoFrameNoResize,
		WindowStyleHasFrameNoResize,
		WindowStyleHasFrameCanResize
	};

	struct GuiComponent;
	struct Image;
	struct Framebuffer;
	struct Window
	{
		int cx = 0, cy = 0;

		int lastClickTime = 0;
		bool doubleClicked = false;
		KeyState mouseLeft;
		KeyState mouseMiddle;
		KeyState mouseRight;
		int mouseX = 0, mouseY = 0;
		int mousePrevX = 0, mousePrevY = 0;
		int mouseDispX = 0, mouseDispY = 0;
		int mouseScroll = 0;
		KeyState keyStates[256];

		void *hWnd = nullptr;

		int frameCount = 0;

		VkSurfaceKHR surface;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		Image *images[2] = {};
		std::shared_ptr<Framebuffer> framebuffers[2];

		VkSemaphore imageAvailable;
		unsigned int imageIndex = 0;
		std::vector<VkCommandBuffer> cbs;
		VkFence frameDone;

		GuiComponent *ui = nullptr;

		bool dead = false;

		virtual void keyDownEvent(int);
		virtual void keyUpEvent(int);
		virtual void charEvent(int);
		virtual void mouseLeftDownEvent(int, int);
		virtual void mouseLeftUpEvent(int, int);
		virtual void mouseMiddleDownEvent(int, int);
		virtual void mouseMiddleUpEvent(int, int);
		virtual void mouseRightDownEvent(int, int);
		virtual void mouseRightUpEvent(int, int);
		virtual void mouseMoveEvent(int, int);
		virtual void mouseWheelEvent(int);
		virtual void renderEvent();

		Window(int _cx, int _cy, HWND _hWnd, bool hasUi = false);
		Window(int _cx, int _cy, const std::string &title, WindowStyle style, bool hasUi = false);
		virtual ~Window();
		void createSwapchain();
		void destroySwapchain();
		int getFPS();
		void beginFrame(bool clearBackground);
		void endFrame();
		void show();
	};

	extern Window* current_window;

	void initWindow();
}
