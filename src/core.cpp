#include <map>

#include "core.h"
#include "event.h"
#include "render.h"
#include "scene.h"
#include "gui.h"
#include "window.h"
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

	CriticalSection renderCs;
	VkSemaphore imageAvailable;
	unsigned int imageIndex = 0;
	VkEvent renderFinished;
	VkFence frameDone;

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

		imageAvailable = vk::createSemaphore();
		renderFinished = vk::createEvent();
		frameDone = vk::createFence();

		initGui();
		guiSetupIcons();

		controllingObject = nullptr;
		initPhysics();

		return Err::eNoErr;
	}

	void beginFrame()
	{
		renderCs.lock();

		imageIndex = vk::acquireNextImage(currentWindow->swapchain, imageAvailable);
	}

	void endFrame()
	{
		vk::waitFence(frameDone);

		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.swapchainCount = 1;
		info.pSwapchains = &currentWindow->swapchain;
		info.pImageIndices = &imageIndex;
		vk::queuePresent(&info);

		renderCs.unlock();
	}

	void mainLoop(Window *p)
	{
		currentWindow = p;

		startUpTime = GetTickCount();

		for (;;)
		{
			if (currentWindow->die)
			{
				delete currentWindow;
				currentWindow = nullptr;
				return;
			}

			nowTime = GetTickCount() - startUpTime;
			processEvents();

			MSG msg;

			auto hasMsg = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			if ((hasMsg && msg.message == WM_QUIT) || currentWindow->die)
			{
				vk::destroySwapchain(currentWindow->surface, currentWindow->swapchain);
				if (msg.message != WM_QUIT)
					DestroyWindow(currentWindow->hWnd); // destroy window if window die cause by code
				return;
			}

			if (hasMsg)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if (currentWindow)
			{
				currentWindow->mouseEvent();
				currentWindow->renderEvent();
				currentWindow->frameCount++;
				currentWindow->leftDown = false; 
				currentWindow->leftUp = false;
				currentWindow->mousePrevX = currentWindow->mouseX;
				currentWindow->mousePrevY = currentWindow->mouseY;
				currentWindow->mouseScroll = 0;
			}
		}
	}
}