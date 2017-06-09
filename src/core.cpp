#include <map>

#include "core.h"
#include "event.h"
#include "render.h"
#include "scene.h"
#include "gui.h"
#include "physics.h"

namespace tke
{
	int resCx;
	int resCy;

	float aspect;

	glm::mat4 matOrtho;
	glm::mat4 matOrthoInv;
	glm::mat4 matPerspective;
	glm::mat4 matPerspectiveInv;
	glm::mat4 *pMatProj;
	glm::mat4 *pMatProjInv;
	bool needUpdateProjMatrix;

	VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineVertexInputStateCreateInfo lineVertexInputState;
	VkPipelineVertexInputStateCreateInfo animatedVertexInputState;

	StagingBuffer *stagingBuffer = nullptr;

	std::string enginePath;

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

	bool needRedraw;

	void changeProjMat(int what)
	{
		if (what == ORTHO)
		{
			pMatProj = &matOrtho;
			pMatProjInv = &matOrthoInv;
		}
		else
		{
			pMatProj = &matPerspective;
			pMatProjInv = &matPerspectiveInv;
		}
		needUpdateProjMatrix;
	}

	int thread_local startUpTime = 0;
	int thread_local nowTime = 0;

	Err init(const char *appName, int rcx, int rcy)
	{
		initRender(appName, 
#ifdef _DEBUG
			true
#else
			false
#endif
		);

		resCx = rcx;
		resCy = rcy;

		matOrtho = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f)) * glm::ortho(-1.f, 1.f, -1.f, 1.f, TKE_NEAR, TKE_FAR * 2);
		matOrthoInv = glm::inverse(matOrtho);
		aspect = (float)resCx / resCy;
		matPerspective = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f)) * glm::perspective(TKE_FOVY, aspect, TKE_NEAR, TKE_FAR);
		matPerspectiveInv = glm::inverse(matPerspective);

		changeProjMat(PERSPECTIVE);

		stagingBuffer = new StagingBuffer(65536);

		scene = new Scene;
		scene->name = "default";
		scene->setUp();

		{
			zeroVertexInputState = vertexStateInfo(0, nullptr, 0, nullptr);

			static VkVertexInputBindingDescription bindings0 = { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

			static VkVertexInputAttributeDescription attributes0[] = {
				{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
				{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) },
				{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
				{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) }
			};

			vertexInputState = vertexStateInfo(1, &bindings0, ARRAYSIZE(attributes0), attributes0);

			static VkVertexInputBindingDescription bindings1 = { 0, sizeof(AnimatedVertex), VK_VERTEX_INPUT_RATE_VERTEX };

			static VkVertexInputAttributeDescription attributes1[] = {
				{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AnimatedVertex, position) },
				{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(AnimatedVertex, uv) },
				{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AnimatedVertex, normal) },
				{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(AnimatedVertex, tangent) },
				{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(AnimatedVertex, boneWeight) },
				{ 5, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(AnimatedVertex, boneID) }
			};

			animatedVertexInputState = vertexStateInfo(1, &bindings1, ARRAYSIZE(attributes1), attributes1);

			static VkVertexInputBindingDescription bindings2 = { 0, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX };

			static VkVertexInputAttributeDescription attributes2 = { 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 };

			lineVertexInputState = vertexStateInfo(1, &bindings2, 1, &attributes2);
		}

		// this kind of depth format would not change depth to 0 ~ 1, which will let to be -1 ~ 1.
		globalResource.setImage(new Image(resCx, resCy, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), "Depth.Image");

		{
			VkAttachmentReference ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
			windowRenderPass = createRenderPass(1, &swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_DONT_CARE), 1, &subpassDesc(1, &ref), 0, nullptr);
		}

		initPhysics();

		return Err::eNoErr;
	}

	VkRenderPass windowRenderPass;

	Window::Window(int _cx, int _cy, const std::string &title, bool hasFrame)
	{
		cx = _cx;
		cy = _cy;

		unsigned int windowStyle;
		if (hasFrame)
		{
			RECT rect = { 0, 0, _cx, _cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			_cx = rect.right - rect.left;
			_cy = rect.bottom - rect.top;

			windowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		}
		else
		{
			windowStyle = WS_POPUP;
		}
		hWnd = CreateWindowA("wndClass", title.c_str(), windowStyle, (screenCx - _cx) / 2, (screenCy - _cy) / 2, _cx, _cy, NULL, NULL, hInst, NULL);

		assert(hWnd);

		VkImage vkImages[2];

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

			uint32_t swapchainImageCount = 0;
			vkGetSwapchainImagesKHR(device.v, swapchain, &swapchainImageCount, nullptr);
			vkGetSwapchainImagesKHR(device.v, swapchain, &swapchainImageCount, vkImages);

			device.cs.unlock();
			inst.cs.unlock();
		}

		images = (Image*)malloc(sizeof(Image) * 2);

		for (int i = 0; i < 2; i++)
		{
			new (&images[i]) Image(Image::eSwapchain, vkImages[i], cx, cy, swapchainFormat);

			std::vector<VkImageView> views;
			views.push_back(images[i].getView(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));
			framebuffers[i] = createFramebuffer(cx, cy, windowRenderPass, views);
		}

		commandPool.create();

		imageAvailable = createSemaphore();
		renderFinished = createEvent();
		frameDone = createFence();
	}

	Window::~Window()
	{
		destroyFence(frameDone);
		destroyEvent(renderFinished);
		destroySemaphore(imageAvailable);
		commandPool.destroy();
		for (int i = 0; i < 2; i++)
			releaseFramebuffer(framebuffers[i]);
		delete images;
		inst.cs.lock();
		device.cs.lock();
		vkDestroySwapchainKHR(device.v, swapchain, nullptr);
		vkDestroySurfaceKHR(inst.v, surface, nullptr);
		device.cs.unlock();
		inst.cs.unlock();
	}

	thread_local Window *currentWindow = nullptr;
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (currentWindow)
		{
			switch (message)
			{
			case WM_LBUTTONDOWN:
			{
				static int lastClickTime = 0;
				if (nowTime - lastClickTime < 300)
					currentWindow->doubleClick = true;
				else
					currentWindow->doubleClick = false;
				lastClickTime = nowTime;
				currentWindow->leftPressing = true;
				currentWindow->leftDown = true;
				currentWindow->leftUp = false;
				currentWindow->mouseX = LOWORD(lParam);
				currentWindow->mouseY = HIWORD(lParam);
				SetCapture(hWnd);
			}
				break;
			case WM_LBUTTONUP:
				currentWindow->leftPressing = false;
				currentWindow->leftDown = false;
				currentWindow->leftUp = true;
				ReleaseCapture();
				break;
			case WM_MBUTTONDOWN:
				currentWindow->middlePressing = true;
				currentWindow->mouseX = LOWORD(lParam);
				currentWindow->mouseY = HIWORD(lParam);
				SetCapture(hWnd);
				break;
			case WM_MBUTTONUP:
				currentWindow->middlePressing = false;
				ReleaseCapture();
				break;
			case WM_RBUTTONDOWN:
				currentWindow->rightPressing = true;
				currentWindow->mouseX = LOWORD(lParam);
				currentWindow->mouseY = HIWORD(lParam);
				SetCapture(hWnd);
				break;
			case WM_RBUTTONUP:
				currentWindow->rightPressing = false;
				ReleaseCapture();
				break;
			case WM_MOUSEMOVE:
				currentWindow->mouseX = (short)LOWORD(lParam);
				currentWindow->mouseY = (short)HIWORD(lParam);
				break;
			case WM_MOUSEWHEEL:
				currentWindow->mouseScroll += (short)HIWORD(wParam);
				break;
			case WM_KEYDOWN:
				currentWindow->keyDownEvent(wParam);
				break;
			case WM_KEYUP:
				currentWindow->keyUpEvent(wParam);
				break;
			case WM_CHAR:
				currentWindow->charEvent(wParam);
				break;
			case WM_DESTROY:
				currentWindow->die = true;
				break;
			}
			currentWindow->extraMsgEvent(hWnd, message, wParam, lParam);
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	struct _WindowInit
	{
		_WindowInit()
		{
			WNDCLASSEXA wcex = {};
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = wndProc;
			wcex.hInstance = hInst;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.lpszClassName = "wndClass";
			RegisterClassExA(&wcex);
		}
	};
	static _WindowInit _windowInit;

	void Window::keyDownEvent(int wParam) {}
	void Window::keyUpEvent(int wParam) {}
	void Window::charEvent(int wParam) {}
	void Window::mouseEvent() {}
	void Window::renderEvent() {}
	LRESULT Window::extraMsgEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { return 0; }

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
	}

	void Window::run(bool *dead)
	{
		ShowWindow(hWnd, SW_NORMAL);

		currentWindow = this;

		startUpTime = GetTickCount();

		for (;;)
		{
			if (state == eSinalToPause)
			{
				state = ePausing;
				while (state != eSinalToRun) Sleep(100);
				state = eRunning;
			}

			nowTime = GetTickCount() - startUpTime;
			processEvents();

			bool hasMsg;

			MSG msg;
			hasMsg = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			if (hasMsg)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (die)
			{
				delete this;
				*dead = true;
				return;
			}

			if (!hasMsg)
			{
				mouseEvent();
				renderEvent();
				frameCount++;
				leftDown = false;
				leftUp = false;
				mousePrevX = mouseX;
				mousePrevY = mouseY;
				mouseScroll = 0;
			}
		}
	}
}
