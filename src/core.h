#ifndef __TKE__
#define __TKE__

#include <atomic>
#include <Windows.h>

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

	struct AnimatedVertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;

		glm::vec4 boneWeight;
		glm::ivec4 boneID;
	};

	extern int resCx;
	extern int resCy;

	extern float aspect;

	extern glm::mat4 matOrtho;
	extern glm::mat4 matOrthoInv;
	extern glm::mat4 matPerspective;
	extern glm::mat4 matPerspectiveInv;
	extern glm::mat4 *pMatProj;
	extern glm::mat4 *pMatProjInv;
	extern bool needUpdateProjMatrix;

	extern VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	extern VkPipelineVertexInputStateCreateInfo animatedVertexInputState;
	extern VkPipelineVertexInputStateCreateInfo vertexInputState;
	extern VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	extern StagingBuffer *stagingBuffer;

	void changeProjMat(int what);

	extern std::string enginePath;

	void setReporter(void(*_reporter)(const std::string &));
	void report(const std::string &str);
	void reportMajorProgress(int progress);
	void reportMinorProgress(int progress);
	void setMajorProgressText(const std::string &str);
	void setMinorProgressText(const std::string &str);
	int majorProgress();
	int minorProgress();
	std::string majorProgressText();
	std::string minorProgressText();

	extern bool needRedraw;

	Err init(const char *appName, int rcx, int rcy);

	extern thread_local int startUpTime;
	extern thread_local int nowTime;

	extern VkRenderPass windowRenderPass;

	struct Window
	{
		enum State
		{
			eRunning,
			ePausing,
			eSinalToPause,
			eSinalToRun
		};

		std::atomic_int32_t state = eRunning;

		int cx = 0, cy = 0;

		bool doubleClick = false;
		bool leftDown = false, leftUp = false;
		bool leftPressing = false, middlePressing = false, rightPressing = false;
		int mouseX = 0, mouseY = 0;
		int mousePrevX = 0, mousePrevY = 0;
		int mouseScroll = 0;

		HWND hWnd = nullptr;

		int frameCount = 0;

		VkSurfaceKHR surface;
		VkSwapchainKHR swapchain;
		Image *images = nullptr;
		Framebuffer *framebuffers[2];
		CommandPool commandPool;

		VkSemaphore imageAvailable;
		unsigned int imageIndex = 0;
		VkEvent renderFinished;
		VkFence frameDone;

		bool die = false;

		virtual void keyDownEvent(int);
		virtual void keyUpEvent(int);
		virtual void charEvent(int);
		virtual void mouseEvent();
		virtual void renderEvent();
		virtual LRESULT extraMsgEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		Window(int _cx, int _cy, const std::string &title, bool hasFrame = true);
		~Window();
		int getFPS();
		void beginFrame();
		void endFrame();
		void run(bool *dead);
	};
}

#endif