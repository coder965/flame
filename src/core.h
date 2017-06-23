#pragma once

#include <atomic>
#define NOMINMAX
#include <Windows.h>

#include "math.h"
#include "render.h"
#include "gui.h"

#define TKE_NEAR (0.1f)
#define TKE_FAR (1000.f)
#define TKE_FOVY (60.f)
#define TKE_MAX_STATIC_OBJECT_COUNT (1024)
#define TKE_MAX_ANIMATED_OBJECT_COUNT (8)
#define TKE_MAX_BONE_COUNT (256)
#define TKE_MAX_LIGHT_COUNT (256)
#define TKE_MAX_MATERIAL_COUNT (256)
#define TKE_MAX_TEXTURE_COUNT (256)
#define TKE_MAX_INDIRECT_COUNT (1024)
#define TKE_ENVR_SIZE_CX (128 * 4)
#define TKE_ENVR_SIZE_CY (128 * 2)
#define TKE_SHADOWMAP_CX (2048)
#define TKE_SHADOWMAP_CY (2048)

namespace tke
{
	extern std::string enginePath;

	extern int resCx;
	extern int resCy;

	extern float aspect;

	extern glm::mat4 matOrtho;
	extern glm::mat4 matOrthoInv;
	extern glm::mat4 matPerspective;
	extern glm::mat4 matPerspectiveInv;
	extern glm::mat4 *pMatProj;
	extern glm::mat4 *pMatProjInv;

	enum ProjectType
	{
		ProjectTypeOrtho,
		ProjectTypePerspective
	};
	void changeProjMat(ProjectType type);

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
		glm::vec4 boneID;
	};

	extern VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	extern VkPipelineVertexInputStateCreateInfo animatedVertexInputState;
	extern VkPipelineVertexInputStateCreateInfo vertexInputState;
	extern VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	extern StagingBuffer *stagingBuffer;

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

	extern Pipeline *mrtPipeline;
	extern Pipeline *mrtAnimPipeline;
	void setMasterRenderer(Renderer *r);

	extern bool needRedraw;
	extern bool needUpdateVertexBuffer;
	extern bool needUpdateMaterialBuffer;
	extern bool needUpdateTexture;

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

	struct MatrixBufferShaderStruct
	{
		glm::mat4 proj;
		glm::mat4 projInv;
		glm::mat4 view;
		glm::mat4 viewInv;
		glm::mat4 projView;
		glm::mat4 projViewRotate;
		glm::vec4 frustumPlanes[6];
		glm::vec2 viewportDim;
	};

	struct MaterialShaderStruct
	{
		unsigned int albedoAlphaCompress;
		unsigned int specRoughnessCompress;

		unsigned int mapIndex;

		unsigned int dummy;
	};

	extern std::vector<Image*> textures;

	extern std::vector<MaterialShaderStruct> materials;

	extern VertexBuffer *staticVertexBuffer;
	extern IndexBuffer *staticIndexBuffer;

	extern VertexBuffer *animatedVertexBuffer;
	extern IndexBuffer *animatedIndexBuffer;

	extern UniformBuffer *constantBuffer;
	extern UniformBuffer *materialBuffer;

	Err init(const std::string &path, int rcx, int rcy);

	extern thread_local int startUpTime;
	extern thread_local int nowTime;

	extern VkRenderPass plainRenderPass;
	extern VkRenderPass plainRenderPass_clear;

	struct Window
	{
		int cx = 0, cy = 0;

		int lastClickTime = 0;
		bool doubleClicked = false;
		bool leftJustDown = false, leftJustUp = false;
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

		VkSemaphore imageAvailable;
		unsigned int imageIndex = 0;
		std::vector<VkEvent> events;
		VkFence frameDone;

		GuiComponent *ui = nullptr;

		bool dead = false;

		virtual void keyDownEvent(int);
		virtual void keyUpEvent(int);
		virtual void charEvent(int);
		virtual void mouseLeftDownEvent(int, int);
		virtual void mouseLeftUpEvent(int, int);
		virtual void mouseMiddleDownEvent(int, int);
		virtual void mouseMiddleUpEvent(int, int);
		virtual void mouseRightDownEvent(int, int);
		virtual void mouseRightUpEvent(int, int);
		virtual void mouseMoveEvent(int, int);
		virtual void mouseWheelEvent(int);
		virtual void renderEvent();

		void update();

		Window(int _cx, int _cy, HWND _hWnd, bool hasUi = false);
		Window(int _cx, int _cy, const std::string &title, bool hasFrame = true, bool hasUi = false, unsigned int windowStyle = 0);
		virtual ~Window();
		void createSwapchain();
		void destroySwapchain();
		void addEvent(VkEvent);
		void removeEvent(VkEvent);
		int getFPS();
		void beginFrame();
		void endFrame();
		void addToList();
	};

	void update();
	void run();
}
