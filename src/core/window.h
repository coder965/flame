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

		std::string title;
		int cx = 0, cy = 0;

		bool doubleClick = false;
		bool leftDown = false, leftUp = false;
		bool leftPressing = false, middlePressing = false, rightPressing = false;
		int mouseX = 0, mouseY = 0;
		int mousePrevX = 0, mousePrevY = 0;
		int mouseScroll = 0;
		bool focus = false;
		std::vector<std::string> dropFiles;

		HWND hWnd = nullptr;

		std::uint32_t frameCount = 0;

		VkSurfaceKHR surface;
		VkSwapchainKHR swapchain;
		Image image[2];
		uint32_t imageIndex;
		VkSemaphore imageAvailable;

		virtual void keyDownEvent(int);
		virtual void keyUpEvent(int);
		virtual void charEvent(int);
		virtual void mouseEvent();
		virtual void renderEvent();
		virtual LRESULT extraMsgEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		Window(int _cx, int _cy, const std::string &_title, unsigned int windowStyle = 0, unsigned int windowStyleEx = 0, bool hasFrame = true);
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
