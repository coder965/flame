#include <assert.h>

#include "window.h"

namespace tke
{
	Window *currentWindow = nullptr;

	Window::Window(int cx, int cy, const char *title,
		unsigned int windowStyle, unsigned int windowStyleEx, bool hasFrame)
	{
		m_cx = cx;
		m_cy = cy;
		m_title = title;

		if (hasFrame)
		{
			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = cx;
			rect.bottom = cy;
			AdjustWindowRect(&rect, WS_CAPTION, false);
			cx = rect.right - rect.left;
			cy = rect.bottom - rect.top;
		}
		hWnd = CreateWindowExA(windowStyleEx, "wndClass", title, windowStyle, (screenCx - cx) / 2, (screenCy - cy) / 2, cx, cy, NULL, NULL, hInst, NULL);

		assert(hWnd);

		VkImage images[2];
		vk::createSwapchain(hWnd, m_cx, m_cy, m_surface, m_swapchain, images);
		for (int i = 0; i < 2; i++)
		{
			m_image[i].type = Image::eSwapchain;
			m_image[i].m_width = cx;
			m_image[i].m_height = cy;
			m_image[i].m_viewType = VK_IMAGE_VIEW_TYPE_2D;
			m_image[i].m_format = vk::swapchainFormat;
			m_image[i].m_image = images[i];
		}

		m_imageAvailable = vk::createSemaphore();
	}

	Window::~Window()
	{
		DestroyWindow(hWnd);
	}

	void Window::perpareFrame()
	{
		m_imageIndex = vk::acquireNextImage(m_swapchain, m_imageAvailable);
	}

	void Window::endFrame(VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &m_imageIndex;
		vk::queuePresent(&presentInfo);
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
		case WM_DESTROY:
			currentWindow->deadEvent();
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

	void Window::initEvent() {}
	void Window::deadEvent() {}
	void Window::keyDownEvent(int wParam) {}
	void Window::keyUpEvent(int wParam) {}
	void Window::charEvent(int wParam) {}
	void Window::mouseEvent() {}
	void Window::renderEvent() {}
	LRESULT Window::extraMsgEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { return 0; }

	void Window::show()
	{
		assert(ready); // must init ui
		currentWindow = this;
		ShowWindow(hWnd, SW_SHOWNORMAL);
		SetForegroundWindow(hWnd);
	}

	int Window::getFPS()
	{
		static auto FPS = 0;
		static auto lastTime = 0;
		static auto lastFrame = 0;

		auto _t = GetTickCount();
		if (_t - lastTime >= 1000)
		{
			FPS = m_frameCount - lastFrame;
			lastFrame = m_frameCount;
			lastTime = _t;
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
				currentWindow->initEvent();
				currentWindow->mouseEvent();
				currentWindow->m_frameCount++;
				currentWindow->renderEvent();
				currentWindow->clearInput();
			}
		}
	}
}
