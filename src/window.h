#ifndef __TKE_WINDOW__
#define __TKE_WINDOW__

#include <vector>
#include <string>
#include <Windows.h>

#include "vk.h"
#include "render.h"

namespace tke
{
	extern VkRenderPass windowRenderPass;

	struct Window
	{
		int cx = 0, cy = 0;

		bool doubleClick = false;
		bool leftDown = false, leftUp = false;
		bool leftPressing = false, middlePressing = false, rightPressing = false;
		int mouseX = 0, mouseY = 0;
		int mousePrevX = 0, mousePrevY = 0;
		int mouseScroll = 0;
		bool focus = false;

		HWND hWnd = nullptr;

		int frameCount = 0;

		VkSurfaceKHR surface;
		VkSwapchainKHR swapchain;
		Image image[2];
		VkFramebuffer framebuffer[2];

		virtual void keyDownEvent(int);
		virtual void keyUpEvent(int);
		virtual void charEvent(int);
		virtual void mouseEvent();
		virtual void renderEvent();
		virtual LRESULT extraMsgEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		~Window();
		void create(int _cx, int _cy, const std::string &title, bool hasFrame = true);
		int getFPS();
		void show();
	};

	extern thread_local Window *currentWindow;
}

#endif
