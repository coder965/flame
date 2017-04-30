#include <assert.h>

#include "window.h"

namespace tke
{
	void createClass(LRESULT CALLBACK dialogProc(HWND, UINT, WPARAM, LPARAM), char *className, int extraWindowSize)
	{
		WNDCLASSEXA wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = dialogProc;
		wcex.cbWndExtra = extraWindowSize;
		wcex.hInstance = hInst;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.lpszClassName = className;
		RegisterClassExA(&wcex);
	}

	HWND createWindow(int x, int y, int cx, int cy, const char *className, const char *title, bool bFrame, bool bCenter, unsigned int style, unsigned int exStyle, HWND parent, HMENU hMenu)
	{
		if (bFrame)
		{
			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = cx;
			rect.bottom = cy;
			AdjustWindowRect(&rect, WS_CAPTION, hMenu != NULL);
			cx = rect.right - rect.left;
			cy = rect.bottom - rect.top;
		}
		if (bCenter)
		{
			x = (screenCx - cx) / 2;
			y = (screenCy - cy) / 2;
		}
		return CreateWindowExA(exStyle, className, title, style, x, y, cx, cy, parent, hMenu, hInst, NULL);
	}

	std::vector<Window *> pWindows;

	Window::Window(int cx, int cy, const char *title,
		unsigned int windowStyle, unsigned int windowStyleEx, bool hasFrame)
	{
		m_cx = cx;
		m_cy = cy;
		m_title = title;

		hWnd = createWindow(0, 0, m_cx, m_cy, "wndClass", m_title.c_str(),
			hasFrame, true, windowStyle, windowStyleEx, NULL, NULL);

		if (!hWnd) return;

		SetWindowLongPtr(hWnd, 0, (LONG_PTR)this);
		pWindows.push_back(this);

		VkImage images[2];
		auto format = vk::createSwapchain(hWnd, m_cx, m_cy, m_surface, m_swapchain, images);
		for (int i = 0; i < 2; i++)
		{
			m_image[i].type = Image::eSwapchain;
			m_image[i].m_width = cx;
			m_image[i].m_height = cy;
			m_image[i].m_viewType = VK_IMAGE_VIEW_TYPE_2D;
			m_image[i].m_format = format;
			m_image[i].m_image = images[i];
		}

		m_imageAvailable = vk::createSemaphore();
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
		auto pWindow = (Window*)GetWindowLongPtr(hWnd, 0);
		if (!pWindow) return DefWindowProc(hWnd, message, wParam, lParam);

		pWindow->receiveInput(hWnd, message, wParam, lParam);

		pWindow->extraMsgEvent(hWnd, message, wParam, lParam);

		LRESULT res = 0;

		switch (message)
		{
		case WM_KEYDOWN:
			pWindow->keyDownEvent(wParam);
			break;
		case WM_KEYUP:
			pWindow->keyUpEvent(wParam);
			break;
		case WM_CHAR:
			pWindow->charEvent(wParam);
			break;
		case WM_DESTROY:
			pWindow->deadCode = 1;
			pWindow->deadEvent();
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
			createClass(wndProc, "wndClass", sizeof(LONG_PTR));
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
		assert(ready);
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
			else
			{
				for (auto it = pWindows.begin(); it != pWindows.end(); )
				{
					if ((*it)->deadCode)
					{
						delete (*it);
						it = pWindows.erase(it);
					}
					else
					{
						it++;
					}
				}

				for (auto pWindow : pWindows)
				{
					if (!pWindow->focus) continue;
					pWindow->initEvent();
					pWindow->mouseEvent();
					pWindow->m_frameCount++;
					pWindow->renderEvent();
					pWindow->clearInput();
				}
			}
		}
	}
}
