#include <map>
#include <regex>
#include <chrono>

#include "global.h"
#include "system.h"
#include "input.h"
#include "global.h"
#include "engine.h"
#include "graphics/buffer.h"
#include "graphics/image.h"
#include "graphics/renderpass.h"
#include "graphics/framebuffer.h"
#include "graphics/synchronization.h"
#include "graphics/renderer.h"
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
	UniformBuffer *constantBuffer = nullptr;

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
	Image *window_images[2];
	std::shared_ptr<Framebuffer> window_framebuffers[2];
	VkSemaphore window_imageAvailable;
	uint32_t window_imageIndex;
	VkFence frameDone;
	PF_EVENT1 onKeyDown = nullptr;
	PF_EVENT1 onKeyUp = nullptr;
	PF_EVENT1 onChar = nullptr;
	PF_EVENT2 onMouseLeftDown = nullptr;
	PF_EVENT2 onMouseLeftUp = nullptr;
	PF_EVENT2 onMouseMiddleDown = nullptr;
	PF_EVENT2 onMouseMiddleUp = nullptr;
	PF_EVENT2 onMouseRightDown = nullptr;
	PF_EVENT2 onMouseRightUp = nullptr;
	PF_EVENT2 onMouseMove = nullptr;
	PF_EVENT1 onMouseWheel = nullptr;
	PF_EVENT0 onRender = nullptr;
	PF_EVENT0 onDestroy = nullptr;
	std::uint32_t window_style;

	long long get_now_time_ms()
	{
		return std::chrono::system_clock::now().time_since_epoch().count() / 10000;
	}

	Node *root_node;

	struct ConstantBufferStruct
	{
		float depth_near;
		float depth_far;
		float cx;
		float cy;
		float aspect;
		float fovy;
		float tanHfFovy;
		float envrCx;
		float envrCy;
	};

	static void _create_swapchain()
	{
		unsigned int physicalDeviceSurfaceFormatCount = 0;
		std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, window_surface, &physicalDeviceSurfaceFormatCount, nullptr);
		physicalDeviceSurfaceFormats.resize(physicalDeviceSurfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, window_surface, &physicalDeviceSurfaceFormatCount, physicalDeviceSurfaceFormats.data());

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
			window_images[i] = new Image(Image::eSwapchain, vkImages[i], window_cx, window_cy, swapchain_format);
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
					vkDestroySwapchainKHR(vk_device.v, swapchain, nullptr);
					_create_swapchain();
				}
				break;
			}
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	int init(bool vulkan_debug, const std::string &path, int rcx, int rcy, int _window_cx, int _window_cy, const std::string &title, unsigned int _window_style, bool _only_2d)
	{
		auto init_start_time = GetTickCount();

		root_node = new Node(NodeTypeNode);

		only_2d = _only_2d;

		SetCurrentDirectory(get_exe_path().c_str());

#ifdef _MSVC_LANG
		SetProcessDPIAware();
#endif

		engine_path = path;
		res_cx = rcx;
		res_cy = rcy;
		res_aspect = (float)res_cx / res_cy;

		initVulkan(vulkan_debug);

		initUi();

		if (!only_2d)
		{
			initModel();
			init_pick_up();

			constantBuffer = new UniformBuffer(sizeof ConstantBufferStruct);
			globalResource.setBuffer(constantBuffer, "Constant.UniformBuffer");

			{
				ConstantBufferStruct stru;
				stru.depth_near = near_plane;
				stru.depth_far = far_plane;
				stru.cx = res_cx;
				stru.cy = res_cy;
				stru.aspect = res_aspect;
				stru.fovy = fovy;
				stru.tanHfFovy = std::tan(glm::radians(fovy * 0.5f));
				stru.envrCx = EnvrSizeCx;
				stru.envrCy = EnvrSizeCy;
				constantBuffer->update(&stru, defalut_staging_buffer);
			}

			initPhysics();
		}

		//initSound();

		window_cx = _window_cx;
		window_cy = _window_cy;

		{
			auto iconData = createImageData("ico.png");

			WNDCLASSEXA wcex = {};
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = _wnd_proc;
			wcex.hInstance = (HINSTANCE)get_hinst();
			wcex.hIcon = CreateIcon(wcex.hInstance, iconData->levels[0].cx,
				iconData->levels[0].cy, 1, 32, nullptr, iconData->levels[0].v.get());
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.lpszClassName = "tke_wnd";
			RegisterClassExA(&wcex);
		}

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
		frameDone = createFence();

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

	struct _Event
	{
		int id;
		EventType type;
		std::function<void()> e;
	};
	std::vector<_Event> _beforeFrameEvents;
	void add_before_frame_event(const std::function<void()> &e, int id, EventType event_type)
	{
		bool dontAdd = false;
		if (id != -1 && event_type == EventTypeOnlyOne)
		{
			for (auto &e : _beforeFrameEvents)
			{
				if (e.id == id)
					return;
			}
		}
		_beforeFrameEvents.push_back({
			id,
			event_type,
			e
			});
	}

	void begin_frame(bool clearBackground)
	{
		for (auto &e : _beforeFrameEvents)
			e.e();
		_beforeFrameEvents.clear();

		auto res = vkAcquireNextImageKHR(vk_device.v, swapchain, UINT64_MAX, window_imageAvailable, VK_NULL_HANDLE, &window_imageIndex);
		assert(res == VK_SUCCESS);

		beginUi(clearBackground);
	}

	static std::vector<VkCommandBuffer> _cbs;

	void add_to_drawlist(VkCommandBuffer cb)
	{
		_cbs.push_back(cb);
	}

	void end_frame()
	{
		endUi();

		if (_cbs.size())
			tke::vk_graphics_queue.submit(_cbs.size(), _cbs.data(), window_imageAvailable, 0, frameDone);
		waitFence(frameDone);

		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.swapchainCount = 1;
		info.pSwapchains = &swapchain;
		info.pImageIndices = &window_imageIndex;

		vk_graphics_queue.mtx.lock();
		auto res = vkQueuePresentKHR(vk_graphics_queue.v, &info);
		assert(res == VK_SUCCESS);
		vk_graphics_queue.mtx.unlock();

		_cbs.clear();
	}

	static unsigned int _lastTime = 0;

	void run()
	{
		for (;;)
		{
			nowTime = GetTickCount();
			static unsigned int frameCount = 0;
			frameCount++;
			total_frame_count++;
			if (nowTime - _lastTime >= 1000)
			{
				FPS = std::max(frameCount, 1U);
				_lastTime = nowTime;
				frameCount = 0;
			}

			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					if (onDestroy)
						onDestroy();
					return;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				mouseDispX = mouseX - mousePrevX;
				mouseDispY = mouseY - mousePrevY;

				root_node->update();
				onRender();

				mouseLeft.justDown = false;
				mouseLeft.justUp = false;
				mouseMiddle.justDown = false;
				mouseMiddle.justUp = false;
				mouseRight.justDown = false;
				mouseRight.justUp = false;
				mousePrevX = mouseX;
				mousePrevY = mouseY;
				mouseScroll = 0;
				for (int i = 0; i < ARRAYSIZE(keyStates); i++)
				{
					keyStates[i].justDown = false;
					keyStates[i].justUp = false;
				}
			}
		}
	}
}
