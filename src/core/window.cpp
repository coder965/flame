#include <assert.h>

#include "window.h"

namespace tke
{
	VkRenderPass windowRenderPass;

	Window::Window(int _cx, int _cy, const std::string &_title, unsigned int windowStyle, unsigned int windowStyleEx, bool hasFrame)
	{
		cx = _cx;
		cy = _cy;
		title = _title;

		if (hasFrame)
		{
			RECT rect = { 0, 0, _cx, _cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			_cx = rect.right - rect.left;
			_cy = rect.bottom - rect.top;
		}
		hWnd = CreateWindowExA(windowStyleEx, "wndClass", title.c_str(), windowStyle, (screenCx - _cx) / 2, (screenCy - _cy) / 2, _cx, _cy, NULL, NULL, hInst, NULL);

		assert(hWnd);

		VkImage images[2];
		vk::createSwapchain(hWnd, cx, cy, surface, swapchain, images);
		for (int i = 0; i < 2; i++)
		{
			image[i].type = Image::eSwapchain;
			image[i].m_width = cx;
			image[i].m_height = cy;
			image[i].m_viewType = VK_IMAGE_VIEW_TYPE_2D;
			image[i].m_format = vk::swapchainFormat;
			image[i].m_image = images[i];

			std::vector<VkImageView> views;
			views.push_back(image[i].getView(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));
			framebuffer[i] = getFramebuffer(cx, cy, windowRenderPass, views);
		}

		imageAvailable = vk::createSemaphore();
	}

	Window::~Window()
	{
		DestroyWindow(hWnd);
	}

	void Window::perpareFrame()
	{
		imageIndex = vk::acquireNextImage(swapchain, imageAvailable);
	}

	void Window::endFrame(VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &waitSemaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &swapchain;
		info.pImageIndices = &imageIndex;
		vk::queuePresent(&info);
	}

	void Window::clearInput()
	{
		leftDown = false; leftUp = false;
		mousePrevX = mouseX;
		mousePrevY = mouseY;
		mouseScroll = 0;
		dropFiles.clear();
	}

	void Window::receiveInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		static DWORD lastClickTime = 0;

		switch (message)
		{
		case WM_LBUTTONDOWN:
		{
			auto nowTime = GetTickCount();
			if (nowTime - lastClickTime < 300)
				doubleClick = true;
			else
				doubleClick = false;
			lastClickTime = nowTime;
			leftPressing = true;
			leftDown = true;
			leftUp = false;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			SetCapture(hWnd);
		}
			break;
		case WM_LBUTTONUP:
			leftPressing = false;
			leftDown = false;
			leftUp = true;
			ReleaseCapture();
			break;
		case WM_MBUTTONDOWN:
			middlePressing = true;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			SetCapture(hWnd);
			break;
		case WM_MBUTTONUP:
			middlePressing = false;
			ReleaseCapture();
			break;
		case WM_RBUTTONDOWN:
			rightPressing = true;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			SetCapture(hWnd);
			break;
		case WM_RBUTTONUP:
			rightPressing = false;
			ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			mouseX = (short)LOWORD(lParam);
			mouseY = (short)HIWORD(lParam);
			break;
		case WM_MOUSEWHEEL:
			mouseScroll += (short)HIWORD(wParam);
			break;
		case WM_ACTIVATE:
			switch (LOWORD(wParam))
			{
			case WA_ACTIVE: case WA_CLICKACTIVE:
				focus = true;
				break;
			case WA_INACTIVE:
				focus = false;
				break;
			}
			break;
		case WM_DROPFILES:
		{
			auto hDrop = (HDROP)wParam;
			auto n = DragQueryFileA(hDrop, (UINT)-1, nullptr, 0);
			for (int i = 0; i < n; i++)
			{
				char buf[MAX_PATH];
				DragQueryFileA(hDrop, i, buf, MAX_PATH);
				dropFiles.push_back(buf);
			}
		}
			break;
		}
	}

	Window *currentWindow = nullptr;

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (!currentWindow) return DefWindowProc(hWnd, message, wParam, lParam);

		currentWindow->receiveInput(hWnd, message, wParam, lParam);
		currentWindow->extraMsgEvent(hWnd, message, wParam, lParam);

		LRESULT res = 0;

		switch (message)
		{
		case WM_KEYDOWN:
			currentWindow->keyDownEvent(wParam);
			break;
		case WM_KEYUP:
			currentWindow->keyUpEvent(wParam);
			break;
		case WM_CHAR:
			currentWindow->charEvent(wParam);
			break;
		default:
			res = DefWindowProc(hWnd, message, wParam, lParam);
		}

		return res;
	}

	struct _WindowInit
	{
		_WindowInit()
		{
			WNDCLASSEXA wcex = {};
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = wndProc;
			wcex.hInstance = hInst;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.lpszClassName = "wndClass";
			RegisterClassExA(&wcex);
		}
	};
	static _WindowInit _windowInit;

	void Window::keyDownEvent(int wParam) {}
	void Window::keyUpEvent(int wParam) {}
	void Window::charEvent(int wParam) {}
	void Window::mouseEvent() {}
	void Window::renderEvent() {}
	LRESULT Window::extraMsgEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { return 0; }

	void Window::show()
	{
		assert(ready); // must init ui
		startUpTime = GetTickCount();
		currentWindow = this;
		ShowWindow(hWnd, SW_SHOWNORMAL);
		SetForegroundWindow(hWnd);
	}

	int Window::getFPS()
	{
		static auto FPS = 0;
		static auto lastTime = 0;
		static auto lastFrame = 0;

		if (nowTime - lastTime >= 1000)
		{
			FPS = frameCount - lastFrame;
			lastFrame = frameCount;
			lastTime = nowTime;
		}
		return FPS;
	}

	void mainLoop()
	{
		assert(currentWindow);

		for (;;)
		{
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if (currentWindow->focus)
			{
				currentWindow->mouseEvent();
				currentWindow->nowTime = GetTickCount() - currentWindow->startUpTime;
				currentWindow->renderEvent();
				currentWindow->frameCount++;
				currentWindow->clearInput();
			}
		}
	}

	void initWindow()
	{
		auto attachment = vk::swapchainAttachment(VK_ATTACHMENT_LOAD_OP_DONT_CARE);

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		VkAttachmentReference ref = {};
		ref.attachment = 0;
		ref.layout = VK_IMAGE_LAYOUT_GENERAL;
		subpass.pColorAttachments = &ref;

		windowRenderPass = vk::createRenderPass(1, &attachment, 1, &subpass, 0, nullptr);
	}
}
