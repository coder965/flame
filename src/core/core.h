#ifndef __TKE__
#define __TKE__

#include "vk.h"
#include "math.h"
#include "render.h"

#define TKE_NEAR (0.1f)
#define TKE_FAR (1000.f)
#define TKE_FOVY (60.f)
#define TKE_MAX_BONE_COUNT (256)
#define TKE_ENVR_SIZE_CX (128 * 4)
#define TKE_ENVR_SIZE_CY (128 * 2)
#define TKE_SHADOWMAP_CX (2048)
#define TKE_SHADOWMAP_CY (2048)

namespace tke
{
	enum
	{
		ORTHO,
		PERSPECTIVE
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	struct VertexAnimation
	{
		glm::vec3 position;
		glm::vec2 texcoord;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	extern int resCx;
	extern int resCy;

	extern float aspect;

	extern float mouseX;
	extern float mouseY;

	extern glm::mat4 matOrtho;
	extern glm::mat4 matOrthoInv;
	extern glm::mat4 matPerspective;
	extern glm::mat4 matPerspectiveInv;
	extern glm::mat4 *pMatProj;
	extern glm::mat4 *pMatProjInv;
	extern bool needUpdateProjMatrix;

	struct Object;
	extern Object *controllingObject;

	extern VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	extern VkPipelineVertexInputStateCreateInfo vertexInputState;
	extern VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	extern StagingBuffer stagingBuffer;

	void changeProjMat(int what);

	void setReporter(void(*_reporter)(const std::string &));
	void report(const std::string &str);
	void setProgressReporter(void(*_reporter)(int, float));
	void reportProgress(int which, float progress);

	void postRedrawRequest();

	Err init(const char *appName, int rcx, int rcy, bool *pNeedRedraw);
}

#endif