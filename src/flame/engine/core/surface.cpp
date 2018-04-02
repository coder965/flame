#include <algorithm>
#include <assert.h>

#include <flame/filesystem.h>
#include <flame/system.h>
#include <flame/image.h>
#include <flame/engine/core/input.h>
#include <flame/engine/core/surface.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/synchronization.h>

namespace flame
{
	Surface *surface;

	static void *hWnd;
	static VkSurfaceKHR vk_surface;
	static VkSwapchainKHR vk_swapchain;

	static std::list<std::function<void(int, int)>> _resize_listeners;

	static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		handle_input_message_win32(hWnd, message, wParam, lParam);

		switch (message)
		{
			case WM_SIZE:
			{
				auto x = std::max((int)LOWORD(lParam), 1);
				auto y = std::max((int)HIWORD(lParam), 1);
				if (x != surface->cx || y != surface->cy)
				{
					surface->cx = x;
					surface->cy = y;
					surface->create_swapchain();
					for (auto &e : _resize_listeners)
						e(x, y);
				}
				break;
			}
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	Surface::Surface(int _cx, int _cy, int _style, const std::string &_title)
	{
		surface = this;

		vk_surface = 0;
		vk_swapchain = 0;

		WNDCLASSEXA wcex;
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = _wnd_proc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = (HINSTANCE)get_hinst();
		if (std::filesystem::exists("ico.png"))
		{
			auto icon_image = load_image("ico.png");
			icon_image->swap_RB();
			wcex.hIcon = CreateIcon(wcex.hInstance, icon_image->cx, icon_image->cy, 1,
				icon_image->bpp, nullptr, icon_image->data);
			release_image(icon_image);
		}
		else
			wcex.hIcon = 0;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = "tke_wnd";
		wcex.hIconSm = wcex.hIcon;
		RegisterClassExA(&wcex);

		title = _title;
		set_window_size(_cx, _cy, _style);

		create_swapchain();

		image_available = createSemaphore();
	}

	void Surface::set_window_size(int _cx, int _cy, int _style)
	{
		if (_cx > 0)
			cx = _cx;
		if (_cy > 0)
			cy = _cy;
		style = _style;

		auto win32_style = WS_VISIBLE;
		if ((style & SurfaceStyleFrame) && !(style & SurfaceStyleFullscreen))
		{
			RECT rect = { 0, 0, cx, cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			_cx = rect.right - rect.left;
			_cy = rect.bottom - rect.top;

			win32_style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

			if (style & SurfaceStyleResizable)
				win32_style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}
		else
		{
			win32_style |= WS_BORDER;
			if (style & SurfaceStyleFullscreen)
			{
				_cx = get_screen_cx();
				_cy = get_screen_cy();
			}
		}

		auto x = (get_screen_cx() - _cx) / 2;
		auto y = (get_screen_cy() - _cy) / 2;
		if (hWnd)
		{
			SetWindowLong((HWND)hWnd, GWL_STYLE, win32_style);
			SetWindowPos((HWND)hWnd, HWND_TOP, x, y, _cx, _cy, SWP_NOZORDER);
		}
		else
		{
			hWnd = CreateWindowA("tke_wnd", title.c_str(), win32_style,
				x, y, _cx, _cy, NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		}

		create_swapchain();
	}

	void Surface::set_window_maximized(bool v)
	{
		ShowWindow((HWND)hWnd, v ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	}

	void Surface::create_swapchain()
	{
		if (vk_swapchain)
			vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
		if (vk_surface)
			vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);

		VkWin32SurfaceCreateInfoKHR surface_info;
		surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surface_info.flags = 0;
		surface_info.pNext = nullptr;
		surface_info.hinstance = (HINSTANCE)get_hinst();
		surface_info.hwnd = (HWND)hWnd;
		vk_chk_res(vkCreateWin32SurfaceKHR(vk_instance, &surface_info, nullptr, &vk_surface));

		VkBool32 surface_supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, 0, vk_surface, &surface_supported);
		assert(surface_supported);

		VkSurfaceCapabilitiesKHR surface_capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &surface_capabilities);
		assert(cx >= surface_capabilities.minImageExtent.width);
		assert(cy >= surface_capabilities.minImageExtent.height);
		assert(cx <= surface_capabilities.maxImageExtent.width);
		assert(cy <= surface_capabilities.maxImageExtent.height);

		unsigned int surface_format_count = 0;
		std::vector<VkSurfaceFormatKHR> surface_formats;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface,
			&surface_format_count, nullptr);
		assert(surface_format_count > 0);
		surface_formats.resize(surface_format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface,
			&surface_format_count, surface_formats.data());

		swapchain_format = surface_formats[0].format;

		VkSwapchainCreateInfoKHR swapchain_info;
		swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_info.flags = 0;
		swapchain_info.pNext = nullptr;
		swapchain_info.surface = vk_surface;
		swapchain_info.minImageCount = 2;
		swapchain_info.imageFormat = swapchain_format;
		swapchain_info.imageColorSpace = surface_formats[0].colorSpace;
		swapchain_info.imageExtent.width = cx;
		swapchain_info.imageExtent.height = cy;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_info.queueFamilyIndexCount = 0;
		swapchain_info.pQueueFamilyIndices = nullptr;
		swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchain_info.clipped = true;
		swapchain_info.oldSwapchain = 0;
		vk_chk_res(vkCreateSwapchainKHR(vk_device, &swapchain_info, nullptr, &vk_swapchain));

		uint imageCount = 0;
		vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &imageCount, nullptr);
		vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &imageCount, images);

		for (int i = 0; i < 2; i++)
			image_views[i] = std::make_unique<TextureView>(images[i], swapchain_format, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void Surface::acquire_image()
	{
		vk_chk_res(vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &image_index));
	}

	void Surface::present(VkSemaphore wait_semaphore)
	{
		VkPresentInfoKHR present_info;
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;
		present_info.pResults = nullptr;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &wait_semaphore;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &vk_swapchain;
		present_info.pImageIndices = &image_index;
		//begin_profile("render");
		vk_chk_res(vkQueuePresentKHR(vk_graphics_queue, &present_info));
		vk_queue_wait_idle();
	}

	void add_resize_listener(const std::function<void(int, int)> &e)
	{
		_resize_listeners.push_back(e);
	}

	void remove_resize_listener(const std::function<void(int, int)> &e)
	{
		for (auto it = _resize_listeners.begin(); it != _resize_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_resize_listeners.erase(it);
				return;
			}
		}
	}
}
