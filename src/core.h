#pragma once

#include "math.h"
#include "render.h"

namespace tke
{
	struct Scene;

	extern int lastTime;
	extern int nowTime;
	extern int timeDisp;

	extern std::string enginePath;

	extern int resCx;
	extern int resCy;

	extern float aspect;

	extern glm::mat4 matOrtho;
	extern glm::mat4 matOrthoInv;
	extern glm::mat4 matPerspective;
	extern glm::mat4 matPerspectiveInv;

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

	struct LineVertex
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	extern VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	extern VkPipelineVertexInputStateCreateInfo plain2dVertexInputState;
	extern VkPipelineVertexInputStateCreateInfo vertexInputState;
	extern VkPipelineVertexInputStateCreateInfo animatedVertexInputState;
	extern VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	extern StagingBuffer *stagingBuffer;
	
	extern bool needUpdateVertexBuffer;
	extern bool needUpdateMaterialBuffer;
	extern bool needUpdateTexture;

	extern std::vector<Image*> textures;
	inline Image *getTexture(const std::string &filename)
	{
		for (auto i : textures)
		{
			if (i->filename == filename)
				return i;
		}
		return nullptr;
	}

	struct MaterialShaderStruct
	{
		unsigned int albedoAlphaCompress;
		unsigned int specRoughnessCompress;

		unsigned int mapIndex;

		unsigned int dummy;
	};

	extern std::vector<Image*> modelTextures;
	extern std::vector<MaterialShaderStruct> modelMaterials;

	extern VertexBuffer *staticVertexBuffer;
	extern IndexBuffer *staticIndexBuffer;

	extern VertexBuffer *animatedVertexBuffer;
	extern IndexBuffer *animatedIndexBuffer;

	extern UniformBuffer *constantBuffer;
	extern UniformBuffer *materialBuffer;

	extern Image *depthImage;
	extern Image *pickUpImage;

	extern RenderPass *plainRenderPass_image8;
	extern RenderPass *plainRenderPass_image8_clear;
	extern RenderPass *plainRenderPass_image16;
	extern RenderPass *plainRenderPass_image16_clear;
	extern RenderPass *plainRenderPass_depth_clear_image8;
	extern RenderPass *plainRenderPass_depth_clear_image8_clear;
	extern RenderPass *plainRenderPass_window;
	extern RenderPass *plainRenderPass_window_clear;

	extern Framebuffer *pickUpFb;

	extern Pipeline *plainPipeline_2d;
	extern Pipeline *plainPipeline_3d;
	extern Pipeline *plainPipeline_3d_normal;
	extern Pipeline *plainPipeline_3d_depth;
	extern Pipeline *plainPipeline_3d_anim_depth;
	extern Pipeline *plainPipeline_3d_normal_depth;
	extern Pipeline *plainPipeline_3d_wire;
	extern Pipeline *plainPipeline_3d_anim_wire;
	extern Pipeline *plainPipeline_3d_line;
	extern int plain3d_bone_pos;

	struct GuiComponent;
	struct Window
	{
		int cx = 0, cy = 0;

		int lastClickTime = 0;
		bool doubleClicked = false;
		bool leftJustDown = false, leftJustUp = false;
		bool leftPressing = false, middlePressing = false, rightPressing = false;
		int mouseX = 0, mouseY = 0;
		int mousePrevX = 0, mousePrevY = 0;
		int mouseDispX = 0, mouseDispY = 0;
		int mouseScroll = 0;

		void *hWnd = nullptr;

		int frameCount = 0;

		VkSurfaceKHR surface;
		VkSwapchainKHR swapchain;
		Image *images = nullptr;
		Framebuffer *framebuffers[2];

		VkSemaphore imageAvailable;
		unsigned int imageIndex = 0;
		std::vector<VkCommandBuffer> cbs;
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

		Window(int _cx, int _cy, HWND _hWnd, bool hasUi = false);
		Window(int _cx, int _cy, const std::string &title, bool hasFrame = true, bool hasUi = false, unsigned int windowStyle = 0);
		virtual ~Window();
		void createSwapchain();
		void destroySwapchain();
		int getFPS();
		void beginFrame();
		void endFrame();
		void show();
	};

	typedef void(*PF_TICK)(int);
	typedef void(*PF_EXEC)();

	struct Event
	{
		PF_TICK tickFunc = nullptr;
		int duration = 1;
		PF_EXEC execFunc = nullptr;

		int currentTime = 0;
	};

	struct EventList
	{
		std::vector<Event> events;
		bool repeat = false;
		int currentEventIndex = 0;
	};

	void addEventList(EventList *);
	void removeEventList(EventList *);
	void processEvents();
	void processCmdLine(const std::string &str, bool record = true);
	unsigned int pickUp(int x, int y, void(*drawCallback)(CommandBuffer*));
	Err init(const std::string &path, int rcx, int rcy);
	void addShowingScene(Scene *s);
	void removeShowingScene(Scene *s);
	void run();
}
