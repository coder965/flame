#include <map>
#include <regex>
#include <chrono>
#include <list>

#include "global.h"
#include "system.h"
#include "input.h"
#include "global.h"
#include "engine.h"
#include "graphics/buffer.h"
#include "graphics/texture.h"
#include "graphics/renderpass.h"
#include "graphics/renderer.h"
#include "graphics/framebuffer.h"
#include "graphics/synchronization.h"
#include "entity/scene.h"
#include "ui/ui.h"
#include "physics/physics.h"
#include "sound/sound.h"
#include "model/model.h"
#include "entity/terrain.h"
#include "pick_up/pick_up.h"
#include "engine.h"

namespace tke
{
	void processCmdLine(const std::string &str, bool record)
	{
		static std::string last_cmd;

		std::string string(str);

		std::regex pat(R"([\w\.]+)");
		std::smatch sm;

		if (std::regex_search(string, sm, pat))
		{
			if (sm[0].str() == "r")
				processCmdLine(last_cmd.c_str(), false);
		}
	}

	void *hWnd;
	int window_cx;
	int window_cy;
	VkSurfaceKHR window_surface;
	VkSwapchainKHR swapchain;
	Texture *window_images[2];
	std::shared_ptr<Framebuffer> window_framebuffers[2];
	VkSemaphore window_imageAvailable;
	uint32_t window_imageIndex;
	VkFence frame_done;

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

	std::uint32_t window_style;

	long long get_now_time_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(
			std::chrono::system_clock::now()
			).time_since_epoch().count();
	}

	static long long last_time_ns;

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

		VkSwapchainCreateInfoKHR swapchainInfo = {};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = window_surface;
		swapchainInfo.minImageCount = 2;
		swapchainInfo.imageFormat = swapchain_format;
		swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchainInfo.imageExtent.width = window_cx;
		swapchainInfo.imageExtent.height = window_cy;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapchainInfo.clipped = true;
		auto res = vkCreateSwapchainKHR(vk_device.v, &swapchainInfo, nullptr, &swapchain);
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
				//PostQuitMessage(0);
				break;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	int init(bool vulkan_debug, const std::string &path, int rcx, int rcy, int _window_cx, int _window_cy, const std::string &title, unsigned int _window_style, bool _only_2d)
	{
		auto init_start_time = GetTickCount();

		root_node = new Node(NodeTypeNode);
		root_node->name = "root";

		only_2d = _only_2d;

		SetCurrentDirectory(get_exe_path().c_str());

#ifdef _MSVC_LANG
		SetProcessDPIAware();
#endif

		engine_path = path;
		resolution.set(rcx, rcy);

		initVulkan(vulkan_debug);

		ui::init();

		if (!only_2d)
		{
			initModel();
			init_pick_up();

			initPhysics();
		}

		//initSound();

		window_cx = _window_cx;
		window_cy = _window_cy;

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
			wcex.hIcon = CreateIcon(wcex.hInstance, icon->get_cx(), icon->get_cy(), 1,
				icon->bpp, nullptr, icon->get_data());
		}
		else
			wcex.hIcon = 0;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = "tke_wnd";
		wcex.hIconSm = wcex.hIcon;
		RegisterClassExA(&wcex);

		window_style = _window_style;
		{
			auto wndProp = getWin32WndProp();
			hWnd = CreateWindowA("tke_wnd", title.c_str(), wndProp.second,
				(get_screen_cx() - wndProp.first.x) / 2,
				(get_screen_cy() - wndProp.first.y) / 2, wndProp.first.x,
				wndProp.first.y, NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		}

		{
			VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
			surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceInfo.hinstance = (HINSTANCE)get_hinst();
			surfaceInfo.hwnd = (HWND)hWnd;
			auto res = vkCreateWin32SurfaceKHR(vk_instance, &surfaceInfo, nullptr,
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

		window_imageAvailable = createSemaphore();
		frame_done = createFence();

		printf("\n=====INFO=====\nengine init finished - %d ms\n==============\n", GetTickCount() - init_start_time);

		return NoErr;
	}

	std::pair<glm::ivec2, unsigned int> getWin32WndProp()
	{
		std::pair<glm::ivec2, unsigned int> result;

		result.second = WS_VISIBLE;

		if (window_style & WindowStyleFrame)
		{
			RECT rect = { 0, 0, window_cx, window_cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			result.first.x = rect.right - rect.left;
			result.first.y = rect.bottom - rect.top;

			result.second |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

			if (window_style & WindowStyleResize)
				result.second |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}
		else
		{
			result.second |= WS_BORDER;
			if (window_style & WindowStyleFullscreen)
			{
				result.first.x = get_screen_cx();
				result.first.y = get_screen_cy();
				window_cx = get_screen_cx();
				window_cy = get_screen_cy();
			}
		}

		return result;
	}

	void begin_frame()
	{
		auto res = vkAcquireNextImageKHR(vk_device.v, swapchain, UINT64_MAX, window_imageAvailable, VK_NULL_HANDLE, &window_imageIndex);
		assert(res == VK_SUCCESS);

		ui::begin();
	}

	static std::vector<VkCommandBuffer> _cbs;

	void add_to_drawlist(VkCommandBuffer cb)
	{
		_cbs.push_back(cb);
	}

	void end_frame()
	{
		ui::end();

		if (_cbs.size())
		{
			tke::vk_graphics_queue.submit(_cbs.size(), _cbs.data(), window_imageAvailable, 0, frame_done);
			wait_fence(frame_done);
		}

		VkPresentInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		info.pResults = nullptr;
		info.waitSemaphoreCount = 0;
		info.pWaitSemaphores = nullptr;
		info.swapchainCount = 1;
		info.pSwapchains = &swapchain;
		info.pImageIndices = &window_imageIndex;

		vk_graphics_queue.mtx.lock();
		auto res = vkQueuePresentKHR(vk_graphics_queue.v, &info);
		assert(res == VK_SUCCESS);
		vk_graphics_queue.mtx.unlock();

		_cbs.clear();
	}

	static long long _last_sec_time;

	void run(PF_EVENT0 on_render)
	{
		assert(on_render);

		now_ns = get_now_time_ns();
		_last_sec_time = now_ns;

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
				on_render();

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
