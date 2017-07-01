#include <map>

#include "core.h"
#include "event.h"
#include "render.h"
#include "scene.h"
#include "gui.h"
#include "physics.h"
#include "sound.h"

namespace tke
{
	std::string enginePath;

	int resCx;
	int resCy;

	float aspect;

	glm::mat4 matOrtho;
	glm::mat4 matOrthoInv;
	glm::mat4 matPerspective;
	glm::mat4 matPerspectiveInv;
	glm::mat4 *pMatProj;
	glm::mat4 *pMatProjInv;

	void changeProjMat(ProjectType type)
	{
		if (type == ProjectTypeOrtho)
		{
			pMatProj = &matOrtho;
			pMatProjInv = &matOrthoInv;
		}
		else
		{
			pMatProj = &matPerspective;
			pMatProjInv = &matPerspectiveInv;
		}
	}

	VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	VkPipelineVertexInputStateCreateInfo plain2dVertexInputState;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineVertexInputStateCreateInfo animatedVertexInputState;
	VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	StagingBuffer *stagingBuffer = nullptr;

	void(*reporter)(const std::string &) = nullptr;
	void setReporter(void(*_reporter)(const std::string &))
	{
		reporter = _reporter;
	}

	void report(const std::string &str)
	{
		printf(str.c_str());
		if (reporter) reporter(str);
	}

	int _majorProgress;
	int _minorProgress;
	std::string _majorProgressText;
	std::string _minorProgressText;
	CriticalSection progress_cs;

	void reportMajorProgress(int progress)
	{
		progress_cs.lock();
		printf("major progress:%d\n", progress);
		_majorProgress = progress;
		progress_cs.unlock();
	}

	void reportMinorProgress(int progress)
	{
		progress_cs.lock();
		printf("minor progress:%d\n", progress);
		_minorProgress = progress;
		progress_cs.unlock();
	}

	void setMajorProgressText(const std::string &str)
	{
		progress_cs.lock();
		printf("%s\n", str.c_str());
		_majorProgressText = str;
		progress_cs.unlock();
	}

	void setMinorProgressText(const std::string &str)
	{
		progress_cs.lock();
		printf("%s\n", str.c_str());
		_minorProgressText = str;
		progress_cs.unlock();
	}

	int majorProgress()
	{
		progress_cs.lock();
		auto progress = _majorProgress;
		progress_cs.unlock();
		return progress;
	}

	int minorProgress()
	{
		progress_cs.lock();
		auto progress = _minorProgress;
		progress_cs.unlock();
		return progress;
	}

	std::string majorProgressText()
	{
		progress_cs.lock();
		auto text = _majorProgressText;
		progress_cs.unlock();
		return text;
	}

	std::string minorProgressText()
	{
		progress_cs.lock();
		auto text = _minorProgressText;
		progress_cs.unlock();
		return text;
	}

	Pipeline *mrtPipeline = nullptr;
	Pipeline *mrtAnimPipeline = nullptr;
	void setMasterRenderer(Renderer *r)
	{
		mrtPipeline = r->resource.getPipeline("Mrt.Pipeline");
		mrtAnimPipeline = r->resource.getPipeline("Mrt_Anim.Pipeline");
	}

	bool needRedraw = true;
	bool needUpdateVertexBuffer = true;
	bool needUpdateMaterialBuffer = true;
	bool needUpdateTexture = true;

	std::vector<Image*> textures;
	std::vector<MaterialShaderStruct> materials;

	VertexBuffer *staticVertexBuffer = nullptr;
	IndexBuffer *staticIndexBuffer = nullptr;

	VertexBuffer *animatedVertexBuffer = nullptr;
	IndexBuffer *animatedIndexBuffer = nullptr;

	UniformBuffer *constantBuffer = nullptr;
	UniformBuffer *materialBuffer = nullptr;

	int thread_local startUpTime = 0;
	int thread_local nowTime = 0;

	VkRenderPass plainRenderPass_window;
	VkRenderPass plainRenderPass_window_clear;
	VkRenderPass plainRenderPass_image8;
	VkRenderPass plainRenderPass_image8_clear;
	VkRenderPass plainRenderPass_image16;
	VkRenderPass plainRenderPass_image16_clear;

	Pipeline *plain2dPipeline = nullptr;
	Pipeline *plain3dPipeline = nullptr;

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

		changeProjMat(ProjectTypePerspective);

		initRender( 
#ifdef _DEBUG
			true
#else
			false
#endif
		);

		stagingBuffer = new StagingBuffer(65536);

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
				static VkVertexInputBindingDescription bindings = { 0, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX };

				static VkVertexInputAttributeDescription attributes = { 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 };

				lineVertexInputState = vertexStateInfo(1, &bindings, 1, &attributes);
			}
		}

		// this kind of depth format would not change depth to 0 ~ 1, which will let to be -1 ~ 1.
		globalResource.setImage(new Image(resCx, resCy, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), "Depth.Image");

		{
			VkAttachmentReference ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
			VkSubpassDescription subpass = subpassDesc(1, &ref);
			plainRenderPass_window = createRenderPass(1, &swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_DONT_CARE), 1, &subpass, 0, nullptr);
			plainRenderPass_window_clear = createRenderPass(1, &swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_CLEAR), 1, &subpass, 0, nullptr);
			plainRenderPass_image8 = createRenderPass(1, &colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_DONT_CARE), 1, &subpass, 0, nullptr);
			plainRenderPass_image8_clear = createRenderPass(1, &colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR), 1, &subpass, 0, nullptr);
			plainRenderPass_image16 = createRenderPass(1, &colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE), 1, &subpass, 0, nullptr);
			plainRenderPass_image16_clear = createRenderPass(1, &colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR), 1, &subpass, 0, nullptr);
		}

		plain2dPipeline = new Pipeline;
		plain2dPipeline->loadXML(enginePath + "pipeline/plain2D/plain2D.xml");
		plain2dPipeline->setup(plainRenderPass_window, 0);

		plain3dPipeline = new Pipeline;
		plain3dPipeline->loadXML(enginePath + "pipeline/plain3D/plain3D.xml");
		plain3dPipeline->setup(plainRenderPass_window, 0);

		initPhysics();

		//initSound();

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
			constantBuffer->update(&stru, *stagingBuffer);
		}

		initGeneralModels();

		return Err::eNoErr;
	}

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
		auto p = (Window*)GetWindowLongPtr(hWnd, 0);

		if (p)
		{
			switch (message)
			{
			case WM_LBUTTONDOWN:
				p->mouseLeftDownEvent(LOWORD(lParam), HIWORD(lParam));
				SetCapture(hWnd);
				break;
			case WM_LBUTTONUP:
				p->mouseLeftUpEvent(LOWORD(lParam), HIWORD(lParam));
				ReleaseCapture();
				break;
			case WM_MBUTTONDOWN:
				p->mouseMiddleDownEvent(LOWORD(lParam), HIWORD(lParam));
				SetCapture(hWnd);
				break;
			case WM_MBUTTONUP:
				p->mouseMiddleUpEvent(LOWORD(lParam), HIWORD(lParam));
				ReleaseCapture();
				break;
			case WM_RBUTTONDOWN:
				p->mouseRightDownEvent(LOWORD(lParam), HIWORD(lParam));
				SetCapture(hWnd);
				break;
			case WM_RBUTTONUP:
				p->mouseRightUpEvent(LOWORD(lParam), HIWORD(lParam));
				ReleaseCapture();
				break;
			case WM_MOUSEMOVE:
				p->mouseMoveEvent(LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_MOUSEWHEEL:
				p->mouseWheelEvent((short)HIWORD(wParam));
				break;
			case WM_KEYDOWN:
				p->keyDownEvent(wParam);
				break;
			case WM_KEYUP:
				p->keyUpEvent(wParam);
				break;
			case WM_CHAR:
				p->charEvent(wParam);
				break;
			case WM_SIZE:
			{
				auto cx = LOWORD(lParam);
				auto cy = HIWORD(lParam);
				if (cx != p->cx || cy != p->cy)
				{
					p->cx = cx;
					p->cy = cy;
					p->destroySwapchain();
					p->createSwapchain();
				}
			}
				break;
			case WM_DESTROY:
				p->dead = true;
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
			wcex.hInstance = hInst;
			wcex.hIcon = CreateIcon(hInst, iconData->cx, iconData->cy, 1, 32, nullptr, iconData->data);
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.lpszClassName = "tke_wnd";
			wcex.cbWndExtra = sizeof(LONG_PTR);
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
		hWnd = CreateWindowA("tke_wnd", title.c_str(), windowStyle, (screenCx - _cx) / 2, (screenCy - _cy) / 2, _cx, _cy, NULL, NULL, hInst, NULL);
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)this);

		_create_window(this, hasUi);
	}

	Window::~Window()
	{
		destroyFence(frameDone);
		for (auto &e : events)
			destroyEvent(e);
		destroySemaphore(imageAvailable);
		for (int i = 0; i < 2; i++)
			releaseFramebuffer(framebuffers[i]);
		destroySwapchain();
	}

	void Window::createSwapchain()
	{
		VkResult res;

		inst.cs.lock();
		device.cs.lock();

		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = hInst;
		surfaceInfo.hwnd = hWnd;
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

		device.cs.unlock();
		inst.cs.unlock();

		images = (Image*)malloc(sizeof(Image) * 2);

		for (int i = 0; i < 2; i++)
		{
			new (&images[i]) Image(Image::eSwapchain, vkImages[i], cx, cy, swapchainFormat);
			framebuffers[i] = getFramebuffer(&images[i], plainRenderPass_window);
		}
	}

	void Window::destroySwapchain()
	{
		delete images;
		inst.cs.lock();
		device.cs.lock();
		vkDestroySwapchainKHR(device.v, swapchain, nullptr);
		vkDestroySurfaceKHR(inst.v, surface, nullptr);
		device.cs.unlock();
		inst.cs.unlock();
	}

	void Window::keyDownEvent(int wParam) 
	{
		if (ui)
			ui->onKeyDown(wParam);
	}

	void Window::keyUpEvent(int wParam) 
	{
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

		leftPressing = true;
		leftJustDown = true;
		leftJustUp = false;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseLeftUpEvent(int x, int y)
	{
		leftPressing = false;
		leftJustDown = false;
		leftJustUp = true;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseMiddleDownEvent(int x, int y)
	{
		middlePressing = true;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseMiddleUpEvent(int x, int y)
	{
		middlePressing = false;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseRightDownEvent(int x, int y)
	{
		rightPressing = true;

		mouseX = x;
		mouseY = y;
	}

	void Window::mouseRightUpEvent(int x, int y)
	{
		rightPressing = false;

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


	void Window::update()
	{
		mouseDispX = mouseX - mousePrevX;
		mouseDispY = mouseY - mousePrevY;

		renderEvent();
		frameCount++;

		leftJustDown = false;
		leftJustUp = false;
		mousePrevX = mouseX;
		mousePrevY = mouseY;
		mouseScroll = 0;
	}

	void Window::pushCB(VkCommandBuffer cb, VkEvent e)
	{
		cbs.push_back(cb);
		events.push_back(e);
	}

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
		device.cs.lock();
		auto res = vkAcquireNextImageKHR(device.v, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);
		assert(res == VK_SUCCESS);
		device.cs.unlock();
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

		graphicsQueue.cs.lock();
		auto res = vkQueuePresentKHR(graphicsQueue.v, &info);
		assert(res == VK_SUCCESS);
		graphicsQueue.cs.unlock();

		cbs.clear();
		events.clear();
	}

	static std::vector<Window*> window_list;

	void Window::addToList()
	{
		ShowWindow(hWnd, SW_NORMAL);
		window_list.push_back(this);
	}

	void update()
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
						if (i < pModel->positions.size()) vertex.position = pModel->positions[i];
						else vertex.position = glm::vec3(0.f);
						if (i < pModel->uvs.size()) vertex.uv = pModel->uvs[i];
						else vertex.uv = glm::vec2(0.f);
						if (i < pModel->normals.size()) vertex.normal = pModel->normals[i];
						else vertex.normal = glm::vec3(0.f);
						if (i < pModel->tangents.size()) vertex.tangent = pModel->tangents[i];
						else vertex.tangent = glm::vec3(0.f);

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
						if (i < pModel->positions.size()) vertex.position = pModel->positions[i];
						else vertex.position = glm::vec3(0.f);
						if (i < pModel->uvs.size()) vertex.uv = pModel->uvs[i];
						else vertex.uv = glm::vec2(0.f);
						if (i < pModel->normals.size()) vertex.normal = pModel->normals[i];
						else vertex.normal = glm::vec3(0.f);
						if (i < pModel->tangents.size()) vertex.tangent = pModel->tangents[i];
						else vertex.tangent = glm::vec3(0.f);

						if (i < pModel->boneWeights.size()) vertex.boneWeight = pModel->boneWeights[i];
						else vertex.boneWeight = glm::vec4(0.f);
						if (i < pModel->boneIDs.size()) vertex.boneID = pModel->boneIDs[i];
						else vertex.boneID = glm::vec4(0.f);

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

			tke::needRedraw = true;
			needUpdateVertexBuffer = false;
		}
		if (needUpdateTexture)
		{
			static int map_position0 = -1;
			static int map_position1 = -1;
			if (map_position0 == -1 && mrtPipeline) map_position0 = mrtPipeline->descriptorPosition("mapSamplers");
			if (map_position1 == -1 && mrtAnimPipeline) map_position1 = mrtAnimPipeline->descriptorPosition("mapSamplers");
			if (map_position0 != -1 && map_position1 != -1)
			{
				for (int index = 0; index < textures.size(); index++)
				{
					descriptorPool->addWrite(mrtPipeline->descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, map_position0, textures[index]->getInfo(colorSampler), index);
					descriptorPool->addWrite(mrtAnimPipeline->descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, map_position1, textures[index]->getInfo(colorSampler), index);
				}
				descriptorPool->update();
				tke::needRedraw = true;
				needUpdateTexture = false;
			}
		}
		if (needUpdateMaterialBuffer)
		{
			if (materials.size() > 0)
				materialBuffer->update(materials.data(), *stagingBuffer, sizeof(MaterialShaderStruct) * materials.size());
			needUpdateMaterialBuffer = false;
		}

		for (auto it = window_list.begin(); it != window_list.end(); )
		{
			auto w = *it;
			if (w->dead)
			{
				delete w;
				it = window_list.erase(it);
			}
			else
			{
				w->update();

				it++;
			}
		}
	}

	void run()
	{
		startUpTime = GetTickCount();

		for (;;)
		{
			nowTime = GetTickCount() - startUpTime;
			processEvents();

			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				update();
			}

			if (window_list.size() == 0)
				return;
		}
	}
}
