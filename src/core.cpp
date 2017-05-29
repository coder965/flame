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

	Object *controllingObject;

	VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	StagingBuffer stagingBuffer;

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
	
	static Image _depthImage;

	Err init(const char *appName, int rcx, int rcy)
	{
		vk::init(appName, 
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

		scene = new Scene;
		strcpy(scene->name, "default");
		scene->setUp();

		stagingBuffer.create(65536);

		{
			static VkVertexInputBindingDescription bindings0[] = {
				{ 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }
			};

			static VkVertexInputAttributeDescription attributes0[] = {
				{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
				{ 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec3) },
				{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2) },
				{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) }
			};

			static VkVertexInputBindingDescription bindings1[] = {
				{ 0, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX }
			};

			static VkVertexInputAttributeDescription attributes1[] = {
				{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 }
			};

			zeroVertexInputState = vk::vertexState(0, nullptr, 0, nullptr);

			vertexInputState = vk::vertexState(ARRAYSIZE(bindings0), bindings0, ARRAYSIZE(attributes0), attributes0);

			lineVertexInputState = vk::vertexState(ARRAYSIZE(bindings1), bindings1, ARRAYSIZE(attributes1), attributes1);
		}

		_depthImage.create(resCx, resCy, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		globalResource.setImage(&_depthImage, "Depth.Image");

		{
			auto attachment = vk::swapchainAttachment(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			VkAttachmentReference ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
			subpass.pColorAttachments = &ref;
			windowRenderPass = vk::createRenderPass(1, &attachment, 1, &subpass, 0, nullptr);
		}

		controllingObject = nullptr;
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
		vk::createSwapchain(hWnd, cx, cy, surface, swapchain, vkImages);
		for (int i = 0; i < 2; i++)
		{
			image[i].type = Image::eSwapchain;
			image[i].m_width = cx;
			image[i].m_height = cy;
			image[i].m_viewType = VK_IMAGE_VIEW_TYPE_2D;
			image[i].m_format = vk::swapchainFormat;
			image[i].m_image = vkImages[i];

			std::vector<VkImageView> views;
			views.push_back(image[i].getView(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1));
			framebuffer[i] = createFramebuffer(cx, cy, windowRenderPass, views);
		}

		commandPool.create();

		imageAvailable = vk::createSemaphore();
		renderFinished = vk::createEvent();
		frameDone = vk::createFence();
	}

	Window::~Window()
	{
		die = false;
		vk::destroyFence(frameDone);
		vk::destroyEvent(renderFinished);
		vk::destroySemaphore(imageAvailable);
		commandPool.destroy();
		for (int i = 0; i < 2; i++)
		{
			destroyFramebuffer(framebuffer[i]);
			image[i].destroy();
		}
		vk::destroySwapchain(surface, swapchain);
		DestroyWindow(hWnd);
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
			case WM_ACTIVATE:
				switch (LOWORD(wParam))
				{
				case WA_ACTIVE: case WA_CLICKACTIVE:
					currentWindow->focus = true;
					break;
				case WA_INACTIVE:
					currentWindow->focus = false;
					break;
				}
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
		imageIndex = vk::acquireNextImage(swapchain, imageAvailable);
	}

	void Window::endFrame()
	{
		vk::waitFence(frameDone);

		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.swapchainCount = 1;
		info.pSwapchains = &swapchain;
		info.pImageIndices = &imageIndex;
		vk::queuePresent(&info);
	}

	void Window::run(bool *dead)
	{
		ShowWindow(hWnd, SW_NORMAL);

		currentWindow = this;

		startUpTime = GetTickCount();

		for (;;)
		{
			if (die)
			{
				delete this;
				*dead = nullptr;
				return;
			}

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