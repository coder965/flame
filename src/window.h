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

	struct Image;
	struct Framebuffer;
	struct Window
	{
		Window(int _cx, int _cy, const std::string &title, WindowStyle style);
		void createSwapchain();
		void beginFrame(bool clearBackground);
		void endFrame();
		void show();
	};

	void initWindow();
}
