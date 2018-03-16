#include <vector>

#include <flame/global.h>
#include <flame/common/filesystem.h>
#include <flame/common/system.h>
#include <flame/engine/core/application.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/framebuffer.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/synchronization.h>
#include <flame/engine/ui/ui.h>
#include <flame/engine/entity/node.h>

namespace flame
{
	static std::list<std::function<void(int)>> _keydown_listeners;
	static std::list<std::function<void(int)>> _keyup_listeners;
	static std::list<std::function<void(int)>> _char_listeners;
	static std::list<std::function<void(int, int)>> _resize_listeners;
	static std::list<std::function<void()>> _destroy_listeners;

	static std::list<std::function<void()>> _after_frame_events;

	void add_keydown_listener(const std::function<void(int)> &e)
	{
		_keydown_listeners.push_back(e);
	}

	void add_keyup_listener(const std::function<void(int)> &e)
	{
		_keyup_listeners.push_back(e);
	}

	void add_char_listener(const std::function<void(int)> &e)
	{
		_char_listeners.push_back(e);
	}

	void add_resize_listener(const std::function<void(int, int)> &e)
	{
		_resize_listeners.push_back(e);
	}

	void add_destroy_listener(const std::function<void()> &e)
	{
		_destroy_listeners.push_back(e);
	}

	void remove_keydown_listener(const std::function<void(int)> &e)
	{
		for (auto it = _keydown_listeners.begin(); it != _keydown_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_keydown_listeners.erase(it);
				return;
			}
		}
	}

	void remove_keyup_listener(const std::function<void(int)> &e)
	{
		for (auto it = _keyup_listeners.begin(); it != _keyup_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_keyup_listeners.erase(it);
				return;
			}
		}
	}

	void remove_char_listener(const std::function<void(int)> &e)
	{
		for (auto it = _char_listeners.begin(); it != _char_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_char_listeners.erase(it);
				return;
			}
		}
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

	void remove_destroy_listener(const std::function<void()> &e)
	{
		for (auto it = _destroy_listeners.begin(); it != _destroy_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_destroy_listeners.erase(it);
				return;
			}
		}
	}

	static std::mutex _after_frame_event_mtx;

	void add_after_frame_event(const std::function<void()> &e)
	{
		_after_frame_event_mtx.lock();
		_after_frame_events.push_back(e);
		_after_frame_event_mtx.unlock();
	}

	Application *app = nullptr;

	static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (app)
		{
			switch (message)
			{
				case WM_LBUTTONDOWN:
					app->on_lbutton_down(LOWORD(lParam), HIWORD(lParam));
					SetCapture(hWnd);
					break;
				case WM_LBUTTONUP:
					app->on_lbutton_up(LOWORD(lParam), HIWORD(lParam));
					ReleaseCapture();
					break;
				case WM_MBUTTONDOWN:
					app->on_mbutton_down(LOWORD(lParam), HIWORD(lParam));
					SetCapture(hWnd);
					break;
				case WM_MBUTTONUP:
					app->on_mbutton_up(LOWORD(lParam), HIWORD(lParam));
					ReleaseCapture();
					break;
				case WM_RBUTTONDOWN:
					app->on_rbutton_down(LOWORD(lParam), HIWORD(lParam));
					SetCapture(hWnd);
					break;
				case WM_RBUTTONUP:
					app->on_rbutton_up(LOWORD(lParam), HIWORD(lParam));
					ReleaseCapture();
					break;
				case WM_MOUSEMOVE:
					app->on_mouse_move(LOWORD(lParam), HIWORD(lParam));
					break;
				case WM_MOUSEWHEEL:
					app->on_mouse_wheel((short)HIWORD(wParam));
					break;
				case WM_KEYDOWN:
					app->on_key_down(wParam);
					break;
				case WM_KEYUP:
					app->on_key_up(wParam);
					break;
				case WM_CHAR:
					app->on_char(wParam);
					break;
				case WM_SIZE:
					app->on_size(LOWORD(lParam), HIWORD(lParam));
					break;
				case WM_DESTROY:
					PostQuitMessage(0);
					break;
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	InputState::InputState()
	{
		reset();
	}

	void InputState::reset()
	{
		just_down = false;
		just_up = false;
		pressing = false;
	}

	void InputState::on_down()
	{
		just_down = true;
		just_up = false;
		pressing = true;
	}

	void InputState::on_up()
	{
		just_down = false;
		just_up = true;
		pressing = false;
	}

	Application::Application(int _window_cx, int _window_cy, int _window_style, const std::string &_window_title) :
		hWnd(0)
	{
		root_node = new Node;
		root_node->name = "root";

		WNDCLASSEXA wcex;
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = _wnd_proc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = (HINSTANCE)get_hinst();
		if (std::filesystem::exists("ico.png"))
		{
			Image icon("ico.png");
			icon.swap_RB();
			wcex.hIcon = CreateIcon(wcex.hInstance, icon.cx, icon.cy, 1,
				icon.bpp, nullptr, icon.data);
		}
		else
			wcex.hIcon = 0;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = "tke_wnd";
		wcex.hIconSm = wcex.hIcon;
		RegisterClassExA(&wcex);

		window_title = _window_title;
		set_window_size(_window_cx, _window_cy, _window_style);

		{
			VkWin32SurfaceCreateInfoKHR surface_info;
			surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surface_info.flags = 0;
			surface_info.pNext = nullptr;
			surface_info.hinstance = (HINSTANCE)get_hinst();
			surface_info.hwnd = (HWND)hWnd;
			chk_vk_res(vkCreateWin32SurfaceKHR(vk_instance, &surface_info, nullptr, &window_surface));

			VkBool32 supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, 0, window_surface, &supported);

			VkSurfaceCapabilitiesKHR surfaceCapabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, window_surface, &surfaceCapabilities);

			int cut = 1;
		}

		create_swapchain();

		image_available = createSemaphore();
		frame_finished = createFence();

		app = this;
	}

	Application::~Application()
	{
		for (auto &e : _destroy_listeners)
			e();
	}

	void Application::on_lbutton_down(int x, int y)
	{
		mouse_button[0].on_down();
		mouseX = x;
		mouseY = y;
	}

	void Application::on_lbutton_up(int x, int y)
	{
		mouse_button[0].on_up();
		mouseX = x;
		mouseY = y;
	}

	void Application::on_mbutton_down(int x, int y)
	{
		mouse_button[2].on_down();
		mouseX = x;
		mouseY = y;
	}

	void Application::on_mbutton_up(int x, int y)
	{
		mouse_button[2].on_up();
		mouseX = x;
		mouseY = y;
	}

	void Application::on_rbutton_down(int x, int y)
	{
		mouse_button[1].on_down();
		mouseX = x;
		mouseY = y;
	}

	void Application::on_rbutton_up(int x, int y)
	{
		mouse_button[1].on_up();
		mouseX = x;
		mouseY = y;
	}

	void Application::on_mouse_move(int x, int y)
	{
		mouseX = x;
		mouseY = y;
	}

	void Application::on_mouse_wheel(int v)
	{
		mouseScroll += v;
	}

	void Application::on_key_down(int v)
	{
		key_states[v].on_down();
		for (auto &e : _keydown_listeners)
			e(v);
	}

	void Application::on_key_up(int v)
	{
		key_states[v].on_up();
		for (auto &e : _keyup_listeners)
			e(v);
	}

	void Application::on_char(int v)
	{
		for (auto &e : _char_listeners)
			e(v);
	}

	void Application::on_size(int x, int y)
	{
		x = glm::max(x, 1);
		y = glm::max(y, 1);
		if (x != window_cx || y != window_cy)
		{
			window_cx = x;
			window_cy = y;
			vkDestroySwapchainKHR(vk_device.v, swapchain, nullptr);
			create_swapchain();
			for (auto &e : _resize_listeners)
				e(x, y);
		}
	}

	void Application::update()
	{
		mouseDispX = mouseX - mousePrevX;
		mouseDispY = mouseY - mousePrevY;

		root_node->update();

		chk_vk_res(vkAcquireNextImageKHR(vk_device.v, swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &window_image_index));

		ui::begin();

		on_render();

		ui::end();

		if (!cbs.empty())
		{
			vk_graphics_queue.submit(cbs.size(), cbs.data(), image_available, 0, frame_finished);
			wait_fence(frame_finished);
			cbs.clear();
		}

		VkPresentInfoKHR present_info;
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;
		present_info.pResults = nullptr;
		present_info.waitSemaphoreCount = 0;
		present_info.pWaitSemaphores = nullptr;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain;
		present_info.pImageIndices = &window_image_index;

		vk_graphics_queue.mtx.lock();
		chk_vk_res(vkQueuePresentKHR(vk_graphics_queue.v, &present_info));
		vk_graphics_queue.mtx.unlock();

		for (auto i = 0; i < 3; i++)
		{
			mouse_button[i].just_down = false;
			mouse_button[i].just_up = false;
		}
		mousePrevX = mouseX;
		mousePrevY = mouseY;
		mouseScroll = 0;
		for (int i = 0; i < TK_ARRAYSIZE(key_states); i++)
			key_states[i].reset();

		_after_frame_event_mtx.lock();
		for (auto &e : _after_frame_events)
			e();
		_after_frame_events.clear();
		_after_frame_event_mtx.unlock();
	}

	Framebuffer *Application::get_curr_framebuffer() const
	{
		return window_framebuffers[window_image_index].get();
	}

	void Application::create_swapchain()
	{
		static bool first = true;
		if (first)
		{
			static unsigned int count = 0;
			static std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats;
			vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, window_surface,
				&count, nullptr);
			physicalDeviceSurfaceFormats.resize(count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, window_surface,
				&count, physicalDeviceSurfaceFormats.data());

			first = false;
		}

		VkSwapchainCreateInfoKHR swapchain_info = {};
		swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_info.flags = 0;
		swapchain_info.pNext = nullptr;
		swapchain_info.surface = window_surface;
		swapchain_info.minImageCount = 2;
		swapchain_info.imageFormat = swapchain_format;
		swapchain_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchain_info.imageExtent.width = window_cx;
		swapchain_info.imageExtent.height = window_cy;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchain_info.clipped = true;
		chk_vk_res(vkCreateSwapchainKHR(vk_device.v, &swapchain_info, nullptr, &swapchain));

		VkImage vkImages[2];
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(vk_device.v, swapchain, &imageCount, nullptr);
		vkGetSwapchainImagesKHR(vk_device.v, swapchain, &imageCount, vkImages);

		for (int i = 0; i < 2; i++)
		{
			window_images[i] = std::make_unique<Texture>(vkImages[i], window_cx, window_cy, swapchain_format);
			window_framebuffers[i] = getFramebuffer(window_images[i].get(), renderPass_window);
		}
	}

	void Application::set_window_size(int cx, int cy, int style)
	{
		if (cx > 0)
			window_cx = cx;
		if (cy > 0)
			window_cy = cy;
		window_style = style;

		auto win32_style = WS_VISIBLE;
		if ((window_style & WindowStyleFrame) && !(window_style & WindowStyleFullscreen))
		{
			RECT rect = { 0, 0, window_cx, window_cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			cx = rect.right - rect.left;
			cy = rect.bottom - rect.top;

			win32_style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

			if (window_style & WindowStyleResizable)
				win32_style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}
		else
		{
			win32_style |= WS_BORDER;
			if (window_style & WindowStyleFullscreen)
			{
				cx = get_screen_cx();
				cy = get_screen_cy();
			}
		}

		auto x = (get_screen_cx() - cx) / 2;
		auto y = (get_screen_cy() - cy) / 2;
		if (hWnd)
		{
			SetWindowLong((HWND)hWnd, GWL_STYLE, win32_style);
			SetWindowPos((HWND)hWnd, HWND_TOP, x, y, cx, cy, SWP_NOZORDER);
		}
		else
		{
			hWnd = CreateWindowA("tke_wnd", window_title.c_str(), win32_style,
				x, y, cx, cy, NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		}
	}

	void Application::set_window_maximized(bool v)
	{
		ShowWindow((HWND)hWnd, v ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	}

	void Application::add_cb(VkCommandBuffer cb)
	{
		cbs.push_back(cb);
	}
}
