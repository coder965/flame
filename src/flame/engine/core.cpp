#include <map>
#include <regex>
#include <list>

#include <flame/global.h>
#include <flame/engine/system.h>
#include <flame/engine/input.h>
#include <flame/engine/core.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/renderer.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/synchronization.h>
#include <flame/entity/scene.h>
#include <flame/ui/ui.h>
#include <flame/physics/physics.h>
#include <flame/sound/sound.h>
#include <flame/model/model.h>
#include <flame/pick_up/pick_up.h>

namespace tke
{
	static std::list<std::function<void(int)>> keydown_listeners;
	static std::list<std::function<void(int)>> keyup_listeners;
	static std::list<std::function<void(int)>> char_listeners;
	static std::list<std::function<void(int, int)>> resize_listeners;
	static std::list<std::function<void()>> destroy_listeners;

	static std::list<std::function<void()>> _after_frame_events;

	void add_keydown_listener(const std::function<void(int)> &e)
	{
		keydown_listeners.push_back(e);
	}

	void add_keyup_listener(const std::function<void(int)> &e)
	{
		keyup_listeners.push_back(e);
	}

	void add_char_listener(const std::function<void(int)> &e)
	{
		char_listeners.push_back(e);
	}

	void add_resize_listener(const std::function<void(int, int)> &e)
	{
		resize_listeners.push_back(e);
	}

	void add_destroy_listener(const std::function<void()> &e)
	{
		destroy_listeners.push_back(e);
	}

	template<typename T, typename... U>
	size_t TK_GET_ADDRESS(std::function<T(U...)> f)
	{
		typedef T(fnType)(U...);
		fnType ** fnPointer = f.template target<fnType*>();
		return (size_t)*fnPointer;
	}

	void remove_keydown_listener(const std::function<void(int)> &e)
	{
		for (auto it = keydown_listeners.begin(); it != keydown_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				keydown_listeners.erase(it);
				return;
			}
		}
	}

	void remove_keyup_listener(const std::function<void(int)> &e)
	{
		for (auto it = keyup_listeners.begin(); it != keyup_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				keyup_listeners.erase(it);
				return;
			}
		}
	}

	void remove_char_listener(const std::function<void(int)> &e)
	{
		for (auto it = char_listeners.begin(); it != char_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				char_listeners.erase(it);
				return;
			}
		}
	}

	void remove_resize_listener(const std::function<void(int, int)> &e)
	{
		for (auto it = resize_listeners.begin(); it != resize_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				resize_listeners.erase(it);
				return;
			}
		}
	}

	void remove_destroy_listener(const std::function<void()> &e)
	{
		for (auto it = destroy_listeners.begin(); it != destroy_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				destroy_listeners.erase(it);
				return;
			}
		}
	}

	void add_after_frame_event(const std::function<void()> &e)
	{
		_after_frame_events.push_back(e);
	}

	Node *root_node;

	static void _create_swapchain()
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
		auto res = vkCreateSwapchainKHR(vk_device.v, &swapchain_info, nullptr, &swapchain);
		assert(res == VK_SUCCESS);

		VkImage vkImages[2];
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(vk_device.v, swapchain, &imageCount, nullptr);
		vkGetSwapchainImagesKHR(vk_device.v, swapchain, &imageCount, vkImages);

		for (int i = 0; i < 2; i++)
		{
			window_images[i] = new Texture(vkImages[i], window_cx, window_cy, swapchain_format);
			window_framebuffers[i] = getFramebuffer(window_images[i], renderPass_window);
		}
	}

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
				SetCapture(hWnd);
				break;
			case WM_LBUTTONUP:
				mouseLeft.pressing = false;
				mouseLeft.justDown = false;
				mouseLeft.justUp = true;
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				ReleaseCapture();
				break;
			case WM_MBUTTONDOWN:
				mouseMiddle.pressing = true;
				mouseMiddle.justDown = true;
				mouseMiddle.justUp = false;
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				SetCapture(hWnd);
				break;
			case WM_MBUTTONUP:
				mouseMiddle.pressing = false;
				mouseMiddle.justDown = false;
				mouseMiddle.justUp = true;
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				ReleaseCapture();
				break;
			case WM_RBUTTONDOWN:
				mouseRight.pressing = true;
				mouseRight.justDown = true;
				mouseRight.justUp = false;
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				SetCapture(hWnd);
				break;
			case WM_RBUTTONUP:
				mouseRight.pressing = false;
				mouseRight.justDown = false;
				mouseRight.justUp = true;
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				ReleaseCapture();
				break;
			case WM_MOUSEMOVE:
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				break;
			case WM_MOUSEWHEEL:
				mouseScroll += (short)HIWORD(wParam);
				break;
			case WM_KEYDOWN:
				keyStates[wParam].pressing = true;
				keyStates[wParam].justDown = true;
				keyStates[wParam].justUp = false;
				for (auto &e : keydown_listeners)
					e(wParam);
				break;
			case WM_KEYUP:
				keyStates[wParam].pressing = false;
				keyStates[wParam].justDown = false;
				keyStates[wParam].justUp = true;
				for (auto &e : keyup_listeners)
					e(wParam);
				break;
			case WM_CHAR:
				for (auto &e : char_listeners)
					e(wParam);
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
					vkDestroySwapchainKHR(vk_device.v, swapchain, nullptr);
					_create_swapchain();
					for (auto &e : resize_listeners)
						e(cx, cy);
				}
				break;
			}
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	EngineInitInfo::EngineInitInfo() :
		resolution_x(1280),
		resolution_y(720),
		window_cx(1280),
		window_cy(720),
		window_style(WindowStyleFrame),
		debug_level(0)
	{
	}

	int init(const std::string &_engine_path, int _resolution_x, int _resolution_y, int debug_level, bool _only_2d)
	{
		auto init_start_time = GetTickCount();

		root_node = new Node;
		root_node->name = "root";

		only_2d = _only_2d;

#ifdef _MSVC_LANG
		SetProcessDPIAware();
#endif

		engine_path = _engine_path;
		shader_path = "src/shader/";
		resolution.set(_resolution_x, _resolution_y);

		initVulkan(debug_level > 0);

		ui::init();

		if (!only_2d)
		{
			initModel();
			init_pick_up();

			initPhysics();
		}

		//initSound();

		WNDCLASSEXA wcex;
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = _wnd_proc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = (HINSTANCE)get_hinst();
		if (std::fs::exists("ico.png"))
		{
			auto icon = std::make_unique<Image>("ico.png");
			wcex.hIcon = CreateIcon(wcex.hInstance, icon->cx, icon->cy, 1,
				icon->bpp, nullptr, icon->data.get());
		}
		else
			wcex.hIcon = 0;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = "tke_wnd";
		wcex.hIconSm = wcex.hIcon;
		RegisterClassExA(&wcex);

		window_title = info.window_title;
		set_window_size(info.window_cx, info.window_cy, info.window_style);

		{
			VkWin32SurfaceCreateInfoKHR surface_info;
			surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surface_info.flags = 0;
			surface_info.pNext = nullptr;
			surface_info.hinstance = (HINSTANCE)get_hinst();
			surface_info.hwnd = (HWND)hWnd;
			auto res = vkCreateWin32SurfaceKHR(vk_instance, &surface_info, nullptr,
				&window_surface);
			assert(res == VK_SUCCESS);

			VkBool32 supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, 0, window_surface,
				&supported);

			VkSurfaceCapabilitiesKHR surfaceCapabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, window_surface,
				&surfaceCapabilities);
		}

		_create_swapchain();

		printf("\n=====INFO=====\nengine init finished - %d ms\n==============\n", GetTickCount() - init_start_time);

		return NoErr;
	}

	void set_window_size(int cx, int cy, int style)
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
			SetWindowLong((HWND)tke::hWnd, GWL_STYLE, win32_style);
			SetWindowPos((HWND)tke::hWnd, HWND_TOP, x, y, cx, cy, SWP_NOZORDER);
		}
		else
		{
			hWnd = CreateWindowA("tke_wnd", window_title.c_str(), win32_style,
				x, y, cx, cy, NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		}
	}

	void set_window_maximized(bool v)
	{
		ShowWindow((HWND)tke::hWnd, v ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	}

	static std::vector<VkCommandBuffer> _cbs;

	void add_to_drawlist(VkCommandBuffer cb)
	{
		_cbs.push_back(cb);
	}

	static long long _last_sec_time;

	void run(PF_EVENT0 on_render)
	{
		assert(on_render);

		now_ns = get_now_time_ns();
		_last_sec_time = now_ns;

		auto image_available = createSemaphore();
		auto frame_finished = createFence();

		static long long last_time_ns;

		for (;;)
		{
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					for (auto &e : destroy_listeners)
						e();
					return;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				last_time_ns = now_ns;
				now_ns = get_now_time_ns();
				elapsed_time = (now_ns - last_time_ns) / 1000000000.0;

				static unsigned int frame_count = 0;
				frame_count++;
				total_frame_count++;
				if (now_ns - _last_sec_time >= 1000000000)
				{
					FPS = std::max(frame_count, 1U);
					_last_sec_time = now_ns;
					frame_count = 0;
				}

				mouseDispX = mouseX - mousePrevX;
				mouseDispY = mouseY - mousePrevY;

				root_node->update();

				uint image_index;
				chk_vk_res(vkAcquireNextImageKHR(vk_device.v, swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &image_index));

				ui::begin();

				on_render();

				ui::end();

				if (_cbs.size())
				{
					tke::vk_graphics_queue.submit(_cbs.size(), _cbs.data(), image_available, 0, frame_finished);
					wait_fence(frame_finished);
				}

				VkPresentInfoKHR present_info;
				present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				present_info.pNext = nullptr;
				present_info.pResults = nullptr;
				present_info.waitSemaphoreCount = 0;
				present_info.pWaitSemaphores = nullptr;
				present_info.swapchainCount = 1;
				present_info.pSwapchains = &swapchain;
				present_info.pImageIndices = &image_index;

				vk_graphics_queue.mtx.lock();
				chk_vk_res(vkQueuePresentKHR(vk_graphics_queue.v, &present_info));
				vk_graphics_queue.mtx.unlock();

				_cbs.clear();

				mouseLeft.justDown = false;
				mouseLeft.justUp = false;
				mouseMiddle.justDown = false;
				mouseMiddle.justUp = false;
				mouseRight.justDown = false;
				mouseRight.justUp = false;
				mousePrevX = mouseX;
				mousePrevY = mouseY;
				mouseScroll = 0;
				for (int i = 0; i < TK_ARRAYSIZE(keyStates); i++)
				{
					keyStates[i].justDown = false;
					keyStates[i].justUp = false;
				}

				for (auto &e : _after_frame_events)
					e();
				_after_frame_events.clear();
			}
		}
	}
}
