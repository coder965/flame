#include "window.h"
#include "render/image.h"
#include "render/framebuffer.h"
#include "ui/ui.h"
#include "render/renderpass.h"
#include "render/synchronization.h"
#include "core.h"

namespace tke
{
	RenderPass *renderPass_window;
	RenderPass *renderPass_window_clear;

	static void _create_window(Window *p, bool hasUi)
	{
		p->createSwapchain();

		p->imageAvailable = createSemaphore();
		p->frameDone = createFence();

		if (hasUi)
			p->ui = new GuiComponent(p);
	}

	Window::Window(int _cx, int _cy, HWND _hWnd, bool hasUi)
		:cx(_cx), cy(_cy), hWnd(_hWnd)
	{
		_create_window(this, hasUi);
	}

	static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (current_window)
		{
			switch (message)
			{
			case WM_LBUTTONDOWN:
				current_window->mouseLeftDownEvent(LOWORD(lParam), HIWORD(lParam));
				SetCapture(hWnd);
				break;
			case WM_LBUTTONUP:
				current_window->mouseLeftUpEvent(LOWORD(lParam), HIWORD(lParam));
				ReleaseCapture();
				break;
			case WM_MBUTTONDOWN:
				current_window->mouseMiddleDownEvent(LOWORD(lParam), HIWORD(lParam));
				SetCapture(hWnd);
				break;
			case WM_MBUTTONUP:
				current_window->mouseMiddleUpEvent(LOWORD(lParam), HIWORD(lParam));
				ReleaseCapture();
				break;
			case WM_RBUTTONDOWN:
				current_window->mouseRightDownEvent(LOWORD(lParam), HIWORD(lParam));
				SetCapture(hWnd);
				break;
			case WM_RBUTTONUP:
				current_window->mouseRightUpEvent(LOWORD(lParam), HIWORD(lParam));
				ReleaseCapture();
				break;
			case WM_MOUSEMOVE:
				current_window->mouseMoveEvent(LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_MOUSEWHEEL:
				current_window->mouseWheelEvent((short)HIWORD(wParam));
				break;
			case WM_KEYDOWN:
				current_window->keyDownEvent(wParam);
				break;
			case WM_KEYUP:
				current_window->keyUpEvent(wParam);
				break;
			case WM_CHAR:
				current_window->charEvent(wParam);
				break;
			case WM_SIZE:
			{
				auto cx = glm::max(LOWORD(lParam), (WORD)1);
				auto cy = glm::max(HIWORD(lParam), (WORD)1);
				if (cx != current_window->cx || cy != current_window->cy)
				{
					current_window->cx = cx;
					current_window->cy = cy;
					current_window->destroySwapchain();
					current_window->createSwapchain();
				}
			}
			break;
			case WM_DESTROY:
				current_window->dead = true;
				break;
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	Window::Window(int _cx, int _cy, const std::string &title, bool hasFrame, bool hasUi, unsigned int windowStyle)
		:cx(_cx), cy(_cy)
	{
		static bool first = true;
		if (first)
		{
			first = false;

			auto iconData = createImageData(enginePath + "misc/ico.ico");

			WNDCLASSEXA wcex = {};
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = _wnd_proc;
			wcex.hInstance = (HINSTANCE)hInst;
			wcex.hIcon = CreateIcon((HINSTANCE)hInst, iconData->levels[0].cx, iconData->levels[0].cy, 1, 32, nullptr, iconData->levels[0].v);
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.lpszClassName = "tke_wnd";
			RegisterClassExA(&wcex);

			delete iconData;
		}

		if (hasFrame)
		{
			RECT rect = { 0, 0, _cx, _cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			_cx = rect.right - rect.left;
			_cy = rect.bottom - rect.top;

			windowStyle |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		}
		else
		{
			windowStyle |= WS_POPUP;
		}
		hWnd = CreateWindowA("tke_wnd", title.c_str(), windowStyle, (screenCx - _cx) / 2, (screenCy - _cy) / 2, _cx, _cy, NULL, NULL, (HINSTANCE)hInst, NULL);

		_create_window(this, hasUi);
	}

	Window::~Window()
	{
		destroyFence(frameDone);
		destroySemaphore(imageAvailable);
		destroySwapchain();
	}

	void Window::createSwapchain()
	{
		VkResult res;

		inst.mtx.lock();
		device.mtx.lock();

		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = (HINSTANCE)hInst;
		surfaceInfo.hwnd = (HWND)hWnd;
		res = vkCreateWin32SurfaceKHR(inst.v, &surfaceInfo, nullptr, &surface);
		assert(res == VK_SUCCESS);

		VkBool32 supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &supported);

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

		unsigned int physicalDeviceSurfaceFormatCount = 0;
		std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &physicalDeviceSurfaceFormatCount, nullptr);
		physicalDeviceSurfaceFormats.resize(physicalDeviceSurfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &physicalDeviceSurfaceFormatCount, physicalDeviceSurfaceFormats.data());

		VkSwapchainCreateInfoKHR swapchainInfo = {};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = surface;
		swapchainInfo.minImageCount = 2;
		swapchainInfo.imageFormat = swapchainFormat;
		swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchainInfo.imageExtent.width = cx;
		swapchainInfo.imageExtent.height = cy;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
		swapchainInfo.clipped = true;
		res = vkCreateSwapchainKHR(device.v, &swapchainInfo, nullptr, &swapchain);
		assert(res == VK_SUCCESS);

		VkImage vkImages[2];
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device.v, swapchain, &imageCount, nullptr);
		vkGetSwapchainImagesKHR(device.v, swapchain, &imageCount, vkImages);

		device.mtx.unlock();
		inst.mtx.unlock();

		images = (Image*)malloc(sizeof(Image) * 2);

		for (int i = 0; i < 2; i++)
		{
			new (&images[i]) Image(Image::eSwapchain, vkImages[i], cx, cy, swapchainFormat);
			framebuffers[i] = getFramebuffer(&images[i], renderPass_window);
		}
	}

	void Window::destroySwapchain()
	{
		delete images;
		inst.mtx.lock();
		device.mtx.lock();
		vkDestroySwapchainKHR(device.v, swapchain, nullptr);
		vkDestroySurfaceKHR(inst.v, surface, nullptr);
		device.mtx.unlock();
		inst.mtx.unlock();
	}

	void Window::keyDownEvent(int wParam)
	{
		keyStates[wParam].pressing = true;
		keyStates[wParam].justDown = true;
		keyStates[wParam].justUp = false;
		if (ui)
			ui->onKeyDown(wParam);
	}

	void Window::keyUpEvent(int wParam)
	{
		keyStates[wParam].pressing = false;
		keyStates[wParam].justDown = false;
		keyStates[wParam].justUp = true;
		if (ui)
			ui->onKeyUp(wParam);
	}

	void Window::charEvent(int wParam)
	{
		if (ui)
			ui->onChar(wParam);
	}

	void Window::mouseLeftDownEvent(int x, int y)
	{
		if (nowTime - lastClickTime < 300)
			doubleClicked = true;
		else
			doubleClicked = false;
		lastClickTime = nowTime;

		mouseLeft.pressing = true;
		mouseLeft.justDown = true;
		mouseLeft.justUp = false;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseLeftUpEvent(int x, int y)
	{
		mouseLeft.pressing = false;
		mouseLeft.justDown = false;
		mouseLeft.justUp = true;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseMiddleDownEvent(int x, int y)
	{
		mouseMiddle.pressing = true;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseMiddleUpEvent(int x, int y)
	{
		mouseMiddle.pressing = false;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseRightDownEvent(int x, int y)
	{
		mouseRight.pressing = true;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseRightUpEvent(int x, int y)
	{
		mouseRight.pressing = false;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseMoveEvent(int x, int y)
	{
		mouseX = x;
		mouseY = y;
	}

	void Window::mouseWheelEvent(int v)
	{
		mouseScroll += v;
	}

	void Window::renderEvent() {}

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

	void Window::beginFrame()
	{
		device.mtx.lock();
		auto res = vkAcquireNextImageKHR(device.v, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	void Window::endFrame()
	{
		tke::graphicsQueue.submit(cbs.size(), cbs.data(), imageAvailable, 0, frameDone);
		waitFence(frameDone);

		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.swapchainCount = 1;
		info.pSwapchains = &swapchain;
		info.pImageIndices = &imageIndex;

		graphicsQueue.mtx.lock();
		auto res = vkQueuePresentKHR(graphicsQueue.v, &info);
		assert(res == VK_SUCCESS);
		graphicsQueue.mtx.unlock();
	}

	void Window::show()
	{
		ShowWindow((HWND)hWnd, SW_NORMAL);
		current_window = this;
	}

	Window* current_window = nullptr;

	void initWindow()
	{
		auto att0 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
		auto att1 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_CLEAR);
		VkAttachmentReference col_ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
		VkSubpassDescription subpass0 = subpassDesc(1, &col_ref);
		renderPass_window = new RenderPass(1, &att0, 1, &subpass0);
		renderPass_window_clear = new RenderPass(1, &att1, 1, &subpass0);
	}
}
