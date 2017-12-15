#include <map>
#include <regex>

#include "core.h"
#include "ui/ui.h"
#include "physics/physics.h"
#include "sound/sound.h"
#include "render/renderpass.h"
#include "render/synchronization.h"
#include "render/renderer.h"

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

	unsigned int pickUp(int x, int y, const std::function<void(CommandBuffer*)> &drawCallback)
	{
		if (x < 0 || y < 0 || x > pickUpImage->levels[0].cx || y > pickUpImage->levels[0].cy)
			return 0;

		auto cb = begineOnceCommandBuffer();
		cb->beginRenderPass(renderPass_depthC_image8C, pickUpFb.get());
		drawCallback(cb);
		cb->endRenderPass();
		endOnceCommandBuffer(cb);

		cb = begineOnceCommandBuffer();
		VkBufferImageCopy range = {};
		range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.imageSubresource.layerCount = 1;
		range.imageOffset.x = x;
		range.imageOffset.y = y;
		range.imageExtent.width = 1;
		range.imageExtent.height = 1;
		range.imageExtent.depth = 1;
		vkCmdCopyImageToBuffer(cb->v, pickUpImage->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer->v, 1, &range);
		endOnceCommandBuffer(cb);

		auto pixel = (unsigned char*)stagingBuffer->map(0, 4);
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);
		stagingBuffer->unmap();

		return index;
	}

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
		swapchainInfo.clipped = true;
		auto res = vkCreateSwapchainKHR(vk_device.v, &swapchainInfo, nullptr, &swapchain);
		assert(res == VK_SUCCESS);

		VkImage vkImages[2];
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(vk_device.v, swapchain, &imageCount, nullptr);
		vkGetSwapchainImagesKHR(vk_device.v, swapchain, &imageCount, vkImages);

		for (int i = 0; i < 2; i++)
		{
			window_images[i] = new Image(Image::eSwapchain, vkImages[i], window_cx, window_cy, swapchainFormat);
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

	static bool _only_2d;

	int init(bool vulkan_debug, const std::string &path, int rcx, int rcy, int _window_cx, int _window_cy, const std::string &title, unsigned int window_style, bool only_2d)
	{
		auto init_start_time = GetTickCount();

		_only_2d = only_2d;

		enginePath = path;
		resCx = rcx;
		resCy = rcy;
		resAspect = (float)resCx / resCy;

		if (!only_2d)
		{
			auto vkTrans = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f),
				glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
			matOrtho = vkTrans * glm::ortho(-1.f, 1.f, -1.f, 1.f, near_plane, far_plane * 2);
			matOrthoInv = glm::inverse(matOrtho);
			matPerspective = vkTrans * glm::perspective(glm::radians(fovy), resAspect, near_plane, far_plane);
			matPerspectiveInv = glm::inverse(matPerspective);
		}

		initVulkan(vulkan_debug);

		stagingBuffer = new StagingBuffer(67108864);

		{
			auto att0 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att1 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_CLEAR);
			VkAttachmentReference col_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
			VkSubpassDescription subpass0 = subpassDesc(1, &col_ref);
			renderPass_window = new RenderPass(1, &att0, 1, &subpass0);
			renderPass_windowC = new RenderPass(1, &att1, 1, &subpass0);

			if (!only_2d)
			{
				auto att2 = colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
				auto att3 = colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR);
				auto att4 = colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
				auto att5 = colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR);
				auto att6 = depthAttachmentDesc(VK_FORMAT_D16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR);
				auto att7 = colorAttachmentDesc(VK_FORMAT_R32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR);
				VkAttachmentReference col_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
				VkAttachmentReference dep_ref0 = {0, VK_IMAGE_LAYOUT_GENERAL};
				VkAttachmentReference dep_ref1 = {1, VK_IMAGE_LAYOUT_GENERAL};
				VkSubpassDescription subpass0 = subpassDesc(1, &col_ref);
				VkSubpassDescription subpass1 = subpassDesc(0, nullptr, &dep_ref0);
				VkSubpassDescription subpass2 = subpassDesc(1, &col_ref, &dep_ref1);
				VkAttachmentDescription atts0[] = {
					att2,
					att6
				};
				VkAttachmentDescription atts1[] = {
					att3,
					att6
				};
				VkAttachmentDescription atts2[] = {
					att7,
					att6
				};
				renderPass_image8 = new RenderPass(1, &att2, 1, &subpass0);
				renderPass_image8C = new RenderPass(1, &att3, 1, &subpass0);
				renderPass_image16 = new RenderPass(1, &att4, 1, &subpass0);
				renderPass_image16C = new RenderPass(1, &att5, 1, &subpass0);
				renderPass_depthC = new RenderPass(1, &att6, 1, &subpass1);
				renderPass_depthC_image8 = new RenderPass(ARRAYSIZE(atts0), atts0, 1, &subpass2);
				renderPass_depthC_image8C = new RenderPass(ARRAYSIZE(atts1), atts1, 1, &subpass2);
				renderPass_depthC_image32fC = new RenderPass(ARRAYSIZE(atts2), atts2, 1, &subpass2);
			}
		}

		initUi();

		if (!only_2d)
		{
			depthImage = new Image(resCx, resCy, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			globalResource.setImage(depthImage, "Depth.Image");

			pickUpImage = new Image(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

			VkImageView views[] = {
				pickUpImage->getView(),
				depthImage->getView()
			};
			pickUpFb = getFramebuffer(resCx, resCy, renderPass_depthC_image8C, ARRAYSIZE(views), views);

			initModel();

			constantBuffer = new UniformBuffer(sizeof ConstantBufferStruct);

			globalResource.setBuffer(constantBuffer, "Constant.UniformBuffer");

			{
				ConstantBufferStruct stru;
				stru.depth_near = near_plane;
				stru.depth_far = far_plane;
				stru.cx = resCx;
				stru.cy = resCy;
				stru.aspect = resAspect;
				stru.fovy = fovy;
				stru.tanHfFovy = std::tan(glm::radians(fovy * 0.5f));
				stru.envrCx = EnvrSizeCx;
				stru.envrCy = EnvrSizeCy;
				constantBuffer->update(&stru, stagingBuffer);
			}

			initPhysics();
		}

		//initSound();

		window_cx = _window_cx;
		window_cy = _window_cy;

		{
			auto iconData = createImageData(enginePath + "misc/ico.ico");

			WNDCLASSEXA wcex = {};
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = _wnd_proc;
			wcex.hInstance = (HINSTANCE)hInst;
			wcex.hIcon = CreateIcon((HINSTANCE)hInst, iconData->levels[0].cx, iconData->levels[0].cy, 1, 32, nullptr, iconData->levels[0].v.get());
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.lpszClassName = "tke_wnd";
			RegisterClassExA(&wcex);
		}

		unsigned int win32WindowStyle = WS_VISIBLE;

		if (window_style & WindowStyleFrame)
		{
			RECT rect = {0, 0, _window_cx, _window_cy};
			AdjustWindowRect(&rect, WS_CAPTION, false);
			_window_cx = rect.right - rect.left;
			_window_cy = rect.bottom - rect.top;

			win32WindowStyle |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

			if (window_style & WindowStyleResize)
				win32WindowStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}
		else
			win32WindowStyle |= WS_POPUP;

		hWnd = CreateWindowA("tke_wnd", title.c_str(), win32WindowStyle, (screenCx - _window_cx) / 2, (screenCy - _window_cy) / 2, _window_cx, _window_cy, NULL, NULL, (HINSTANCE)hInst, NULL);

		{
			VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
			surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceInfo.hinstance = (HINSTANCE)hInst;
			surfaceInfo.hwnd = (HWND)hWnd;
			auto res = vkCreateWin32SurfaceKHR(vk_instance, &surfaceInfo, nullptr, &window_surface);
			assert(res == VK_SUCCESS);

			VkBool32 supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, window_surface, &supported);

			VkSurfaceCapabilitiesKHR surfaceCapabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, window_surface, &surfaceCapabilities);
		}

		_create_swapchain();

		window_imageAvailable = createSemaphore();
		frameDone = createFence();

		printf("engine init finished - %d ms\n", GetTickCount() - init_start_time);

		return NoErr;
	}

	struct _Event
	{
		int id;
		EventType type;
		std::function<void()> e;
	};
	std::vector<_Event> _beforeFrameEvents;
	void addBeforeFrameEvent(const std::function<void()> &e, int id, EventType event_type)
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

	void beginFrame(bool clearBackground)
	{
		for (auto &e : _beforeFrameEvents)
			e.e();
		_beforeFrameEvents.clear();

		auto res = vkAcquireNextImageKHR(vk_device.v, swapchain, UINT64_MAX, window_imageAvailable, VK_NULL_HANDLE, &window_imageIndex);
		assert(res == VK_SUCCESS);

		beginUi(clearBackground);
	}

	void FrameCommandBufferList::add(VkCommandBuffer cb, VkEvent e)
	{
		cbs.push_back(cb);
		last_event = e;
	}

	FrameCommandBufferList *addFrameCommandBufferList()
	{
		auto l = new FrameCommandBufferList;
		frameCbLists.push_back(std::move(std::unique_ptr<FrameCommandBufferList>(l)));
		return l;
	}

	void endFrame()
	{
		endUi();

		{
			std::vector<VkCommandBuffer> cbs;
			for (auto &l : frameCbLists)
			{
				for (auto c : l->cbs)
					cbs.push_back(c);
			}
			tke::graphicsQueue.submit(cbs.size(), cbs.data(), window_imageAvailable, 0, frameDone);
		}
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

		frameCbLists.clear();
	}

	static unsigned int _lastTime = 0;

	void run()
	{
		for (;;)
		{
			nowTime = GetTickCount();
			static unsigned int frameCount = 0;
			frameCount++;
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
