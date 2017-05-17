#ifndef __TKE_WINDOW__
#define __TKE_WINDOW__

#include <vector>
#include <string>
#include <Windows.h>

#include "vk.h"
#include "render.h"

namespace tke
{
	struct Window
	{
		bool ready = true;

		std::string m_title;
		int m_cx, m_cy;

		bool doubleClick = false;
		bool leftDown = false, leftUp = false;
		bool leftPressing = false, middlePressing = false, rightPressing = false;
		int mouseX, mouseY;
		int mousePrevX, mousePrevY;
		int mouseScroll;
		bool focus = false;
		std::vector<std::string> dropFiles;

		HWND hWnd = nullptr;

		std::uint32_t m_frameCount = 0;

		VkSurfaceKHR m_surface;
		VkSwapchainKHR m_swapchain;
		Image m_image[2];
		uint32_t m_imageIndex;
		VkSemaphore m_imageAvailable;

		virtual void keyDownEvent(int);
		virtual void keyUpEvent(int);
		virtual void charEvent(int);
		virtual void mouseEvent();
		virtual void renderEvent();
		virtual LRESULT extraMsgEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		Window(int cx, int cy, const char *title, unsigned int windowStyle = 0, unsigned int windowStyleEx = 0, bool hasFrame = true);
		~Window();
		void receiveInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		void clearInput();
		void show();
		int getFPS();
		void perpareFrame();
		void endFrame(VkSemaphore waitSemaphore);
	};

	extern Window *currentWindow;

	void mainLoop();
}

#endif
