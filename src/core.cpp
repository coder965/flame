#include <map>
#include <regex>
#define NOMINMAX
#include <Windows.h>

#include "define.h"
#include "core.h"
#include "gui.h"
#include "entity.h"
#include "physics.h"
#include "sound.h"

namespace tke
{
	int lastTime = 0;
	int nowTime = 0;
	int timeDisp;

	std::string enginePath;

	int resCx;
	int resCy;

	float aspect;

	glm::mat4 matOrtho;
	glm::mat4 matOrthoInv;
	glm::mat4 matPerspective;
	glm::mat4 matPerspectiveInv;

	VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	VkPipelineVertexInputStateCreateInfo plain2dVertexInputState;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineVertexInputStateCreateInfo animatedVertexInputState;
	VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	StagingBuffer *stagingBuffer = nullptr;

	bool needUpdateVertexBuffer = true;
	bool needUpdateMaterialBuffer = true;
	bool needUpdateTexture = true;

	std::vector<std::pair<std::string, Image*>> debugImages;

	std::vector<Image*> textures;

	std::vector<Image*> modelTextures;

	Image *addModelTexture(const std::string &filename, bool sRGB)
	{
		for (auto i : modelTextures)
		{
			if (i->full_filename == filename)
				return i;
		}
		auto i = createImage(filename, sRGB);
		i->index = modelTextures.size();
		modelTextures.push_back(i);
		return i;
	}

	std::vector<Material*> modelMaterials;
	Material *defaultMaterial = nullptr;
	Material *addModelMaterial(unsigned char albedoR, unsigned char albedoG, unsigned char albedoB, unsigned char alpha,
		unsigned char spec, unsigned char roughness, Image *albedoAlphaMap, Image *normalHeightMap, Image *specRoughnessMap)
	{
		for (auto m : modelMaterials)
		{
			if (m->albedoAlphaMap != albedoAlphaMap ? false : (!albedoAlphaMap && m->albedoR == albedoR && m->albedoG == albedoG && m->albedoB == albedoB && m->alpha == alpha)
				&& m->specRoughnessMap != specRoughnessMap ? false : (!specRoughnessMap && m->spec == spec && m->roughness == roughness)
				&& m->normalHeightMap == normalHeightMap)
				return m;
		}
		auto m = new Material;
		m->albedoR = albedoR;
		m->albedoG = albedoG;
		m->albedoB = albedoB;
		m->alpha = alpha;
		m->spec = spec;
		m->roughness = roughness;
		m->albedoAlphaMap = albedoAlphaMap;
		m->normalHeightMap = normalHeightMap;
		m->specRoughnessMap = specRoughnessMap;
		m->sceneIndex = modelMaterials.size();
		modelMaterials.push_back(m);
		return m;
	}

	VertexBuffer *staticVertexBuffer = nullptr;
	IndexBuffer *staticIndexBuffer = nullptr;

	VertexBuffer *animatedVertexBuffer = nullptr;
	IndexBuffer *animatedIndexBuffer = nullptr;

	UniformBuffer *constantBuffer = nullptr;
	UniformBuffer *materialBuffer = nullptr;

	Image *plainDepthImage = nullptr;
	Image *pickUpImage = nullptr;

	RenderPass *renderPass_image8;
	RenderPass *renderPass_image8_clear;
	RenderPass *renderPass_image16;
	RenderPass *renderPass_image16_clear;
	RenderPass *renderPass_depth_clear;
	RenderPass *renderPass_depth_clear_image8;
	RenderPass *renderPass_depth_clear_image8_clear;
	RenderPass *renderPass_depth_clear_image32f_clear;
	RenderPass *renderPass_window;
	RenderPass *renderPass_window_clear;

	Framebuffer *pickUpFb = nullptr;

	Pipeline *plainPipeline_2d = nullptr;
	Pipeline *plainPipeline_3d = nullptr;
	Pipeline *plainPipeline_3d_anim = nullptr;
	Pipeline *plainPipeline_3d_normal = nullptr;
	Pipeline *plainPipeline_3d_tex = nullptr;
	Pipeline *plainPipeline_3d_anim_tex = nullptr;
	Pipeline *plainPipeline_3d_wire = nullptr;
	Pipeline *plainPipeline_3d_anim_wire = nullptr;
	Pipeline *plainPipeline_3d_line = nullptr;
	int plain3d_bone_pos = -1;

	DescriptorSet *ds_maps = nullptr;

	static Window* current_window = nullptr;

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
			wcex.hIcon = CreateIcon((HINSTANCE)hInst, iconData->cx, iconData->cy, 1, 32, nullptr, iconData->v);
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
		for (int i = 0; i < 2; i++)
			releaseFramebuffer(framebuffers[i]);
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
	std::vector<EventList*> eventLists;

	void addEventList(EventList *p)
	{
		eventLists.push_back(p);
	}

	void removeEventList(EventList *p)
	{
		for (auto it = eventLists.begin(); it != eventLists.end(); it++)
		{
			if (*it == p)
			{
				eventLists.erase(it);
				return;
			}
		}
	}

	void processEvents()
	{
		for (auto it = eventLists.begin(); it != eventLists.end(); )
		{
			auto list = *it;

			if (list->currentEventIndex >= list->events.size())
			{
				if (list->repeat)
				{
					list->currentEventIndex = 0;
				}
				else
				{
					delete list;
					it = eventLists.erase(it);
					continue;
				}
				it++;
				continue;
			}

			Event &e = list->events[list->currentEventIndex];
			e.currentTime += timeDisp;
			if (e.tickFunc) e.tickFunc(e.currentTime);

			if (e.currentTime >= e.duration)
			{
				if (e.execFunc) e.execFunc();
				list->currentEventIndex++;
			}

			it++;
		}
	}

	void processCmdLine(const std::string &str, bool record)
	{
		static std::string last_cmd;

		std::string string(str);

		std::regex pat(R"([\w\.]+)");
		std::smatch sm;

		if (std::regex_search(string, sm, pat))
		{
			if (sm[0].str() == "r")
			{
				processCmdLine(last_cmd.c_str(), false);
			}
		}
	}

	unsigned int pickUp(int x, int y, void(*drawCallback)(CommandBuffer*))
	{
		if (x < 0 || y < 0 || x > pickUpImage->cx || y > pickUpImage->cy)
			return 0;

		auto cb = commandPool->begineOnce();
		cb->beginRenderPass(renderPass_depth_clear_image8_clear, pickUpFb);
		drawCallback(cb);
		cb->endRenderPass();
		commandPool->endOnce(cb);

		cb = commandPool->begineOnce();
		VkBufferImageCopy range = {};
		range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.imageSubresource.layerCount = 1;
		range.imageOffset.x = x;
		range.imageOffset.y = y;
		range.imageExtent.width = 1;
		range.imageExtent.height = 1;
		range.imageExtent.depth = 1;
		vkCmdCopyImageToBuffer(cb->v, pickUpImage->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer->v, 1, &range);
		commandPool->endOnce(cb);

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

	struct MaterialShaderStruct
	{
		unsigned int albedoAlphaCompress;
		unsigned int specRoughnessCompress;

		unsigned int mapIndex;

		unsigned int dummy;
	};

	Err init(const std::string &path, int rcx, int rcy)
	{
		enginePath = path;

		resCx = rcx;
		resCy = rcy;

		matOrtho = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f)) * glm::ortho(-1.f, 1.f, -1.f, 1.f, TKE_NEAR, TKE_FAR * 2);
		matOrthoInv = glm::inverse(matOrtho);
		aspect = (float)resCx / resCy;
		matPerspective = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f)) * glm::perspective(TKE_FOVY, aspect, TKE_NEAR, TKE_FAR);
		matPerspectiveInv = glm::inverse(matPerspective);

		initRender(
#if defined(_DEBUG)
			true
#else
			false
#endif
		);

		defaultMaterial = new Material;
		defaultMaterial->sceneIndex = 0;
		modelMaterials.push_back(defaultMaterial);

		stagingBuffer = new StagingBuffer(67108864);

		{
			zeroVertexInputState = vertexStateInfo(0, nullptr, 0, nullptr);

			{
				static VkVertexInputBindingDescription bindings = { 0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX };

				static VkVertexInputAttributeDescription attributes[] = {
					{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos) },
					{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv) },
					{ 2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col) }
				};

				plain2dVertexInputState = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);
			}

			{
				static VkVertexInputBindingDescription bindings = { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

				static VkVertexInputAttributeDescription attributes[] = {
					{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
					{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) },
					{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
					{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) }
				};

				vertexInputState = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);
			}

			{
				static VkVertexInputBindingDescription bindings = { 0, sizeof(AnimatedVertex), VK_VERTEX_INPUT_RATE_VERTEX };

				static VkVertexInputAttributeDescription attributes[] = {
					{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AnimatedVertex, position) },
					{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(AnimatedVertex, uv) },
					{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AnimatedVertex, normal) },
					{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AnimatedVertex, tangent) },
					{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(AnimatedVertex, boneWeight) },
					{ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(AnimatedVertex, boneID) }
				};

				animatedVertexInputState = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);
			}

			{
				static VkVertexInputBindingDescription bindings = { 0, sizeof(LineVertex), VK_VERTEX_INPUT_RATE_VERTEX };

				static VkVertexInputAttributeDescription attributes[] = {
					{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(LineVertex, position) },
					{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(LineVertex, color) }
				};

				lineVertexInputState = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);
			}
		}

		plainDepthImage = new Image(resCx, resCy, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		globalResource.setImage(plainDepthImage, "Depth.Image");

		pickUpImage = new Image(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

		{
			auto att0 = colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att1 = colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att2 = colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att3 = colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att4 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att5 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att6 = depthAttachmentDesc(VK_FORMAT_D16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att7 = colorAttachmentDesc(VK_FORMAT_R32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR);
			VkAttachmentReference col_ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
			VkAttachmentReference dep_ref0 = { 0, VK_IMAGE_LAYOUT_GENERAL };
			VkAttachmentReference dep_ref1 = { 1, VK_IMAGE_LAYOUT_GENERAL };
			VkSubpassDescription subpass0 = subpassDesc(1, &col_ref);
			VkSubpassDescription subpass1 = subpassDesc(0, nullptr, &dep_ref0);
			VkSubpassDescription subpass2 = subpassDesc(1, &col_ref, &dep_ref1);
			VkAttachmentDescription atts0[] = {
				att0,
				att6
			};
			VkAttachmentDescription atts1[] = {
				att1,
				att6
			};
			VkAttachmentDescription atts2[] = {
				att7,
				att6
			};
			renderPass_image8 = new RenderPass(1, &att0, 1, &subpass0);
			renderPass_image8_clear = new RenderPass(1, &att1, 1, &subpass0);
			renderPass_image16 = new RenderPass(1, &att2, 1, &subpass0);
			renderPass_image16_clear = new RenderPass(1, &att3, 1, &subpass0);
			renderPass_depth_clear = new RenderPass(1, &att6, 1, &subpass1);
			renderPass_depth_clear_image8 = new RenderPass(ARRAYSIZE(atts0), atts0, 1, &subpass2);
			renderPass_depth_clear_image8_clear = new RenderPass(ARRAYSIZE(atts1), atts1, 1, &subpass2);
			renderPass_depth_clear_image32f_clear = new RenderPass(ARRAYSIZE(atts2), atts2, 1, &subpass2);
			renderPass_window = new RenderPass(1, &att4, 1, &subpass0);
			renderPass_window_clear = new RenderPass(1, &att5, 1, &subpass0);
		}

		{
			VkImageView views[] = {
				pickUpImage->getView(),
				plainDepthImage->getView()
			};
			pickUpFb = getFramebuffer(resCx, resCy, renderPass_depth_clear_image8_clear, ARRAYSIZE(views), views);
		}

		plainPipeline_2d = new Pipeline;
		plainPipeline_2d->loadXML(enginePath + "pipeline/plain2d/plain2d.xml");
		plainPipeline_2d->setup(renderPass_image8, 0, true);

		plainPipeline_3d = new Pipeline;
		plainPipeline_3d->loadXML(enginePath + "pipeline/plain3d/plain3d.xml");
		plainPipeline_3d->setup(renderPass_depth_clear_image8, 0, false);
		plainPipeline_3d_anim = new Pipeline;
		plainPipeline_3d_anim->loadXML(enginePath + "pipeline/plain3d/plain3d_anim.xml");
		plainPipeline_3d_anim->setup(renderPass_depth_clear_image8, 0, true);
		plainPipeline_3d_normal = new Pipeline;
		plainPipeline_3d_normal->loadXML(enginePath + "pipeline/plain3d/plain3d_normal.xml");
		plainPipeline_3d_normal->setup(renderPass_depth_clear_image8, 0, false);
		plainPipeline_3d_tex = new Pipeline;
		plainPipeline_3d_tex->loadXML(enginePath + "pipeline/plain3d/plain3d_tex.xml");
		plainPipeline_3d_tex->setup(renderPass_depth_clear_image8, 0, false);
		plainPipeline_3d_anim_tex = new Pipeline;
		plainPipeline_3d_anim_tex->loadXML(enginePath + "pipeline/plain3d/plain3d_anim_tex.xml");
		plainPipeline_3d_anim_tex->setup(renderPass_depth_clear_image8, 0, true);
		plainPipeline_3d_wire = new Pipeline;
		plainPipeline_3d_wire->loadXML(enginePath + "pipeline/plain3d/plain3d_wire.xml");
		plainPipeline_3d_wire->setup(renderPass_image8, 0, false);
		plainPipeline_3d_anim_wire = new Pipeline;
		plainPipeline_3d_anim_wire->loadXML(enginePath + "pipeline/plain3d/plain3d_anim_wire.xml");
		plainPipeline_3d_anim_wire->setup(renderPass_image8, 0, true);
		plainPipeline_3d_line = new Pipeline;
		plainPipeline_3d_line->loadXML(enginePath + "pipeline/plain3d/plain3d_line.xml");
		plainPipeline_3d_line->setup(renderPass_image8, 0, false);
		plain3d_bone_pos = plainPipeline_3d_anim_wire->descriptorPosition("BONE");

		staticVertexBuffer = new VertexBuffer();
		staticIndexBuffer = new IndexBuffer();

		animatedVertexBuffer = new VertexBuffer();
		animatedIndexBuffer = new IndexBuffer();

		constantBuffer = new UniformBuffer(sizeof ConstantBufferStruct);
		materialBuffer = new UniformBuffer(sizeof(MaterialShaderStruct) * TKE_MAX_MATERIAL_COUNT);

		globalResource.setBuffer(staticVertexBuffer, "Static.VertexBuffer");
		globalResource.setBuffer(staticIndexBuffer, "Static.IndexBuffer");

		globalResource.setBuffer(animatedVertexBuffer, "Animated.VertexBuffer");
		globalResource.setBuffer(animatedIndexBuffer, "Animated.IndexBuffer");

		globalResource.setBuffer(constantBuffer, "Constant.UniformBuffer");
		globalResource.setBuffer(materialBuffer, "Material.UniformBuffer");

		{
			ConstantBufferStruct stru;
			stru.depth_near = TKE_NEAR;
			stru.depth_far = TKE_FAR;
			stru.cx = resCx;
			stru.cy = resCy;
			stru.aspect = aspect;
			stru.fovy = TKE_FOVY;
			stru.tanHfFovy = std::tan(glm::radians(TKE_FOVY * 0.5f));
			stru.envrCx = TKE_ENVR_SIZE_CX;
			stru.envrCy = TKE_ENVR_SIZE_CY;
			constantBuffer->update(&stru, stagingBuffer);
		}

		initScene();
		initGeneralModels();
		initPhysics();
		//initSound();

		return Err::eNoErr;
	}

	void run()
	{
		lastTime = GetTickCount();

		ds_maps = new DescriptorSet(descriptorPool, mrtPipeline, 1);

		for (;;)
		{
			nowTime = GetTickCount();
			timeDisp = nowTime - lastTime;
			processEvents();

			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				if (needUpdateVertexBuffer)
				{
					std::vector<Vertex> staticVertexs;
					std::vector<int> staticIndices;

					std::vector<AnimatedVertex> animatedVertexs;
					std::vector<int> animatedIndices;

					for (auto pModel : models)
					{
						if (!pModel->animated)
						{
							pModel->vertexBase = staticVertexs.size();
							pModel->indiceBase = staticIndices.size();

							for (int i = 0; i < pModel->positions.size(); i++)
							{
								Vertex vertex;
								vertex.position = i < pModel->positions.size() ? pModel->positions[i] : glm::vec3(0.f);
								vertex.uv       = i < pModel->uvs.size()       ? pModel->uvs[i]       : glm::vec2(0.f);
								vertex.normal   = i < pModel->normals.size()   ? pModel->normals[i]   : glm::vec3(0.f);
								vertex.tangent  = i < pModel->tangents.size()  ? pModel->tangents[i]  : glm::vec3(0.f);

								staticVertexs.push_back(vertex);
							}
							for (int i = 0; i < pModel->indices.size(); i++)
							{
								staticIndices.push_back(pModel->indices[i]);
							}
						}
						else
						{
							pModel->vertexBase = animatedVertexs.size();
							pModel->indiceBase = animatedIndices.size();

							for (int i = 0; i < pModel->positions.size(); i++)
							{
								AnimatedVertex vertex;
								vertex.position   = i < pModel->positions.size()   ? pModel->positions[i]   : glm::vec3(0.f);
								vertex.uv         = i < pModel->uvs.size()         ? pModel->uvs[i]         : glm::vec2(0.f);
								vertex.normal     = i < pModel->normals.size()     ? pModel->normals[i]     : glm::vec3(0.f);
								vertex.tangent    = i < pModel->tangents.size()    ? pModel->tangents[i]    : glm::vec3(0.f);
								vertex.boneWeight = i < pModel->boneWeights.size() ? pModel->boneWeights[i] : glm::vec4(0.f);
								vertex.boneID     = i < pModel->boneIDs.size()     ? pModel->boneIDs[i]     : glm::vec4(0.f);

								animatedVertexs.push_back(vertex);
							}
							for (int i = 0; i < pModel->indices.size(); i++)
							{
								animatedIndices.push_back(pModel->indices[i]);
							}
						}
					}

					if (staticVertexs.size() > 0) staticVertexBuffer->recreate(sizeof(Vertex) * staticVertexs.size(), staticVertexs.data());
					if (staticIndices.size() > 0) staticIndexBuffer->recreate(sizeof(int) * staticIndices.size(), staticIndices.data());

					if (animatedVertexs.size() > 0) animatedVertexBuffer->recreate(sizeof(AnimatedVertex) * animatedVertexs.size(), animatedVertexs.data());
					if (animatedIndices.size() > 0) animatedIndexBuffer->recreate(sizeof(int) * animatedIndices.size(), animatedIndices.data());

					needUpdateVertexBuffer = false;
				}
				if (needUpdateTexture)
				{
					static int map_position = -1;
					if (map_position == -1 && mrtPipeline) map_position = mrtPipeline->descriptorPosition("maps");
					if (map_position != -1)
					{
						for (int index = 0; index < modelTextures.size(); index++)
							ds_maps->setImage(map_position, index, modelTextures[index], colorSampler);
						needUpdateTexture = false;
					}
				}
				if (needUpdateMaterialBuffer)
				{
					if (modelMaterials.size() > 0)
					{
						std::unique_ptr<MaterialShaderStruct[]> mts(new MaterialShaderStruct[modelMaterials.size()]);

						for (int i = 0; i < modelMaterials.size(); i++)
						{
							auto m = modelMaterials[i];

							MaterialShaderStruct mt;
							mts[i].albedoAlphaCompress = m->albedoR + (m->albedoG << 8) + (m->albedoB << 16) + (m->alpha << 24);
							mts[i].specRoughnessCompress = m->spec + (m->roughness << 8);
							mts[i].mapIndex = (m->albedoAlphaMap ? m->albedoAlphaMap->index + 1 : 0) +
								((m->normalHeightMap ? m->normalHeightMap->index + 1 : 0) << 8) +
								((m->specRoughnessMap ? m->specRoughnessMap->index + 1 : 0) << 16);
						}

						materialBuffer->update(mts.get(), stagingBuffer, sizeof(MaterialShaderStruct) * modelMaterials.size());
					}
					needUpdateMaterialBuffer = false;
				}

				if (current_window->dead)
				{
					delete current_window;
					return;
				}
				else
				{
					current_window->mouseDispX = current_window->mouseX - current_window->mousePrevX;
					current_window->mouseDispY = current_window->mouseY - current_window->mousePrevY;

					current_window->renderEvent();
					current_window->frameCount++;

					current_window->mouseLeft.justDown = false;
					current_window->mouseLeft.justUp = false;
					current_window->mousePrevX = current_window->mouseX;
					current_window->mousePrevY = current_window->mouseY;
					current_window->mouseScroll = 0;
				}
			}
			lastTime = nowTime;
		}
	}
}
