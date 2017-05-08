#include <map>
#include "scene.h"
#include "gui.h"
#include "core.h"
#include "physics.h"

namespace tke
{
	int resCx;
	int resCy;

	float aspect;

	float mouseX;
	float mouseY;

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

	void(*progressReporter)(int, float) = nullptr;
	void setProgressReporter(void(*_reporter)(int, float))
	{
		progressReporter = _reporter;
	}

	void reportProgress(int which, float progress)
	{
		printf("%d %f", which, progress);
		if (progressReporter) progressReporter(which, progress);
	}

	bool *_pNeedRedraw = nullptr;
	void postRedrawRequest()
	{
		if (_pNeedRedraw) *_pNeedRedraw = true;
	}

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
	
	static Image _depthImage;

	Err init(const char *appName, int rcx, int rcy, bool *pNeedRedraw)
	{
		static bool first = true;
		if (!first) return Err::eNoErr;
		first = false;

		vk::init(appName, true);

		resCx = rcx;
		resCy = rcy;
		_pNeedRedraw = pNeedRedraw;

		matOrtho = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f)) * glm::ortho(-1.f, 1.f, -1.f, 1.f, TKE_NEAR, TKE_FAR * 2);
		matOrthoInv = glm::inverse(matOrtho);
		aspect = (float)resCx / resCy;
		matPerspective = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f)) * glm::perspective(TKE_FOVY, aspect, TKE_NEAR, TKE_FAR);
		matPerspectiveInv = glm::inverse(matPerspective);

		changeProjMat(PERSPECTIVE);

		scene = new Scene;
		strcpy(scene->name, "default");
		scene->pSunLight = nullptr;
		scene->pSunLight = new Light;
		scene->pSunLight->type = Light::Type::eParallax;
		scene->addLight(scene->pSunLight);
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


		UI::init();
		UI::setupIcons(vk::colorSampler);

		controllingObject = nullptr;

		iniPhysics();

		return Err::eNoErr;
	}
}
