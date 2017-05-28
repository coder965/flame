#include <assert.h>

#include "window.h"
#include "core.h"

namespace tke
{
	VkRenderPass windowRenderPass;

	thread_local Window *currentWindow = nullptr;

	void Window::create(int _cx, int _cy, const std::string &title, bool hasFrame)
	{
		cx = _cx;
		cy = _cy;

		unsigned int windowStyle;
		if (hasFrame)
		{
			RECT rect = { 0, 0, _cx, _cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			_cx = rect.right - rect.left;
			_cy = rect.bottom - rect.top;

			windowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		}
		else
		{
			windowStyle = WS_POPUP;
		}
		hWnd = CreateWindowA("wndClass", title.c_str(), windowStyle, (screenCx - _cx) / 2, (screenCy - _cy) / 2, _cx, _cy, NULL, NULL, hInst, NULL);

		assert(hWnd);

		VkImage vkImages[2];
		vk::createSwapchain(hWnd, cx, cy, surface, swapchain, vkImages);
		for (int i = 0; i < 2; i++)
		{
			image[i].type = Image::eSwapchain;
			image[i].m_width = cx;
			image[i].m_height = cy;
			image[i].m_viewType = VK_IMAGE_VIEW_TYPE_2D;
			image[i].m_format = vk::swapchainFormat;
			image[i].m_image = vkImages[i];

			std::vector<VkImageView> views;
			views.push_back(image[i].getView(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));
			framebuffer[i] = getFramebuffer(cx, cy, windowRenderPass, views);
		}
	}

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (currentWindow)
		{
			switch (message)
			{
			case WM_LBUTTONDOWN:
			{
				static int lastClickTime = 0;
				if (nowTime - lastClickTime < 300)
					currentWindow->doubleClick = true;
				else
					currentWindow->doubleClick = false;
				lastClickTime = nowTime;
				currentWindow->leftPressing = true;
				currentWindow->leftDown = true;
				currentWindow->leftUp = false;
				currentWindow->mouseX = LOWORD(lParam);
				currentWindow->mouseY = HIWORD(lParam);
				SetCapture(hWnd);
			}
				break;
			case WM_LBUTTONUP:
				currentWindow->leftPressing = false;
				currentWindow->leftDown = false;
				currentWindow->leftUp = true;
				ReleaseCapture();
				break;
			case WM_MBUTTONDOWN:
				currentWindow->middlePressing = true;
				currentWindow->mouseX = LOWORD(lParam);
				currentWindow->mouseY = HIWORD(lParam);
				SetCapture(hWnd);
				break;
			case WM_MBUTTONUP:
				currentWindow->middlePressing = false;
				ReleaseCapture();
				break;
			case WM_RBUTTONDOWN:
				currentWindow->rightPressing = true;
				currentWindow->mouseX = LOWORD(lParam);
				currentWindow->mouseY = HIWORD(lParam);
				SetCapture(hWnd);
				break;
			case WM_RBUTTONUP:
				currentWindow->rightPressing = false;
				ReleaseCapture();
				break;
			case WM_MOUSEMOVE:
				currentWindow->mouseX = (short)LOWORD(lParam);
				currentWindow->mouseY = (short)HIWORD(lParam);
				break;
			case WM_MOUSEWHEEL:
				currentWindow->mouseScroll += (short)HIWORD(wParam);
				break;
			case WM_KEYDOWN:
				currentWindow->keyDownEvent(wParam);
				break;
			case WM_KEYUP:
				currentWindow->keyUpEvent(wParam);
				break;
			case WM_CHAR:
				currentWindow->charEvent(wParam);
				break;
			case WM_ACTIVATE:
				switch (LOWORD(wParam))
				{
				case WA_ACTIVE: case WA_CLICKACTIVE:
					currentWindow->focus = true;
					break;
				case WA_INACTIVE:
					currentWindow->focus = false;
					break;
				}
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			}
			currentWindow->extraMsgEvent(hWnd, message, wParam, lParam);
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
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

	void Window::show()
	{
		ShowWindow(hWnd, SW_NORMAL);
	}
}
