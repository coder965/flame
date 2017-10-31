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

	static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_LBUTTONDOWN:
			mouseLeft.pressing = true;
			mouseLeft.justDown = true;
			mouseLeft.justUp = false;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			if (onMouseLeftDown)
				onMouseLeftDown(mouseX, mouseY);
			SetCapture(hWnd);
			break;
		case WM_LBUTTONUP:
			mouseLeft.pressing = false;
			mouseLeft.justDown = false;
			mouseLeft.justUp = true;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			if (onMouseLeftUp)
				onMouseLeftUp(mouseX, mouseY);
			ReleaseCapture();
			break;
		case WM_MBUTTONDOWN:
			mouseMiddle.pressing = true;
			mouseMiddle.justDown = true;
			mouseMiddle.justUp = false;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			if (onMouseMiddleDown)
				onMouseMiddleDown(mouseX, mouseY);
			SetCapture(hWnd);
			break;
		case WM_MBUTTONUP:
			mouseMiddle.pressing = false;
			mouseMiddle.justDown = false;
			mouseMiddle.justUp = true;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			if (onMouseMiddleUp)
				onMouseMiddleUp(mouseX, mouseY);
			ReleaseCapture();
			break;
		case WM_RBUTTONDOWN:
			mouseRight.pressing = true;
			mouseRight.justDown = true;
			mouseRight.justUp = false;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			if (onMouseRightDown)
				onMouseRightDown(mouseX, mouseY);
			SetCapture(hWnd);
			break;
		case WM_RBUTTONUP:
			mouseRight.pressing = false;
			mouseRight.justDown = false;
			mouseRight.justUp = true;
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			if (onMouseRightUp)
				onMouseRightUp(mouseX, mouseY);
			ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			if (onMouseMove)
				onMouseMove(mouseX, mouseY);
			break;
		case WM_MOUSEWHEEL:
			mouseScroll += (short)HIWORD(wParam);
			if (onMouseWheel)
				onMouseWheel(mouseScroll);
			break;
		case WM_KEYDOWN:
			keyStates[wParam].pressing = true;
			keyStates[wParam].justDown = true;
			keyStates[wParam].justUp = false;
			ui_onKeyDown(wParam);
			if (onKeyDown)
				onKeyDown(wParam);
			break;
		case WM_KEYUP:
			keyStates[wParam].pressing = false;
			keyStates[wParam].justDown = false;
			keyStates[wParam].justUp = true;
			ui_onKeyUp(wParam);
			if (onKeyUp)
				onKeyUp(wParam);
			break;
		case WM_CHAR:
			ui_onChar(wParam);
			break;
		case WM_SIZE:
		{
			auto cx = std::max(LOWORD(lParam), (WORD)1);
			auto cy = std::max(HIWORD(lParam), (WORD)1);
			if (cx != window_cx || cy != window_cy)
			{
				window_cx = cx;
				window_cy = cy;
				for (int i = 0; i < 2; i++)
				{
					delete window_images[i];
					window_images[i] = nullptr;
				}
				current_window->createSwapchain();
			}
		}
			break;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	Window::Window(int _cx, int _cy, const std::string &title, WindowStyle style)
	{
		window_cx = _cx;
		window_cy = _cy;

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

		unsigned int win32WindowStyle = WS_VISIBLE;

		if (style != WindowStyleNoFrameNoResize)
		{
			RECT rect = { 0, 0, _cx, _cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			_cx = rect.right - rect.left;
			_cy = rect.bottom - rect.top;

			win32WindowStyle |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

			if (style == WindowStyleHasFrameCanResize)
				win32WindowStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}
		else
			win32WindowStyle |= WS_POPUP;

		hWnd = CreateWindowA("tke_wnd", title.c_str(), win32WindowStyle, (screenCx - _cx) / 2, (screenCy - _cy) / 2, _cx, _cy, NULL, NULL, (HINSTANCE)hInst, NULL);

		{
			VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
			surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceInfo.hinstance = (HINSTANCE)hInst;
			surfaceInfo.hwnd = (HWND)hWnd;
			auto res = vkCreateWin32SurfaceKHR(inst.v, &surfaceInfo, nullptr, &window_surface);
			assert(res == VK_SUCCESS);

			VkBool32 supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, window_surface, &supported);

			VkSurfaceCapabilitiesKHR surfaceCapabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, window_surface, &surfaceCapabilities);
		}

		createSwapchain();

		window_imageAvailable = createSemaphore();
		frameDone = createFence();
	}

	void Window::createSwapchain()
	{
		inst.mtx.lock();
		device.mtx.lock();

		unsigned int physicalDeviceSurfaceFormatCount = 0;
		std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, window_surface, &physicalDeviceSurfaceFormatCount, nullptr);
		physicalDeviceSurfaceFormats.resize(physicalDeviceSurfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, window_surface, &physicalDeviceSurfaceFormatCount, physicalDeviceSurfaceFormats.data());

		VkSwapchainCreateInfoKHR swapchainInfo = {};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = window_surface;
		swapchainInfo.minImageCount = 2;
		swapchainInfo.imageFormat = swapchainFormat;
		swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchainInfo.imageExtent.width = window_cx;
		swapchainInfo.imageExtent.height = window_cy;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchainInfo.oldSwapchain = swapchain;
		swapchainInfo.clipped = true;
		auto res = vkCreateSwapchainKHR(device.v, &swapchainInfo, nullptr, &swapchain);
		assert(res == VK_SUCCESS);

		VkImage vkImages[2];
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device.v, swapchain, &imageCount, nullptr);
		vkGetSwapchainImagesKHR(device.v, swapchain, &imageCount, vkImages);

		device.mtx.unlock();
		inst.mtx.unlock();

		for (int i = 0; i < 2; i++)
		{
			window_images[i] = new Image(Image::eSwapchain, vkImages[i], window_cx, window_cy, swapchainFormat);
			window_framebuffers[i] = getFramebuffer(window_images[i], renderPass_window);
		}
	}

	void Window::beginFrame(bool clearBackground)
	{
		device.mtx.lock();
		auto res = vkAcquireNextImageKHR(device.v, swapchain, UINT64_MAX, window_imageAvailable, VK_NULL_HANDLE, &window_imageIndex);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		beginUi(clearBackground);
	}

	void Window::endFrame()
	{
		endUi();
		cbs.push_back(ui_cb->v);

		tke::graphicsQueue.submit(cbs.size(), cbs.data(), window_imageAvailable, 0, frameDone);
		waitFence(frameDone);

		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.swapchainCount = 1;
		info.pSwapchains = &swapchain;
		info.pImageIndices = &window_imageIndex;

		graphicsQueue.mtx.lock();
		auto res = vkQueuePresentKHR(graphicsQueue.v, &info);
		assert(res == VK_SUCCESS);
		graphicsQueue.mtx.unlock();

		cbs.clear();
		ui_waitEvents.clear();
	}

	void Window::show()
	{
		current_window = this;
	}

	Window* current_window = nullptr;

	void initWindow()
	{
		auto att0 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
		auto att1 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_CLEAR);
		VkAttachmentReference col_ref = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkSubpassDescription subpass0 = subpassDesc(1, &col_ref);
		renderPass_window = new RenderPass(1, &att0, 1, &subpass0);
		renderPass_window_clear = new RenderPass(1, &att1, 1, &subpass0);
	}
}
