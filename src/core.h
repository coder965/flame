#pragma once

#include <functional>

#include "refl.h"
#include "utils.h"
#include "render/buffer.h"
#include "render/image.h"
#include "render/command_buffer.h"

enum
{
	// buffer
	ConstantBufferBinding,
	MaterialBufferBinding,
	MatrixBufferBinding,
	StaticObjectMatrixBufferBinding,
	AnimatedObjectMatrixBufferBinding = 3,
	TerrainBufferBinding,
	WaterBufferBinding,
	LightBufferBinding,
	AmbientBufferBinding,
	ShadowBufferBinding,
	// image
	EnvrImageBinding,
	MainImageBinding,
	DepthImageBinding,
	AlbedoAlphaImageBinding,
	NormalHeightImageBinding,
	SpecRoughnessImageBinding,
	ShadowImageBinding,
	AoImageBinding,
	// terrain special
	TerrainNormalHeightMapBinding,
	TerrainBlendMapBinding,
	TerrainColorMapsBinding,
	TerrainNormalMapsBinding
};

enum
{
	TexturesBindingSet = 1,
	BoneBindingSet
};

namespace tke
{
	IMPL(0.1f) float near_plane;
	IMPL(1000.f) float far_plane;
	IMPL(60.f) float fovy;

	IMPL() int nowTime;

	IMPL() std::string enginePath;
	IMPL() int resCx;
	IMPL() int resCy;
	IMPL() float resAspect;

	IMPL() glm::mat4 matOrtho;
	IMPL() glm::mat4 matOrthoInv;
	IMPL() glm::mat4 matPerspective;
	IMPL() glm::mat4 matPerspectiveInv;

	IMPL(nullptr) StagingBuffer *stagingBuffer;

	IMPL(nullptr) UniformBuffer *constantBuffer;

	IMPL(nullptr) Image *depthImage;
	IMPL(nullptr) Image *pickUpImage;

	IMPL(nullptr) RenderPass *renderPass_image8;
	IMPL(nullptr) RenderPass *renderPass_image8C;
	IMPL(nullptr) RenderPass *renderPass_image16;
	IMPL(nullptr) RenderPass *renderPass_image16C;
	IMPL(nullptr) RenderPass *renderPass_depthC;
	IMPL(nullptr) RenderPass *renderPass_depthC_image8;
	IMPL(nullptr) RenderPass *renderPass_depthC_image8C;
	IMPL(nullptr) RenderPass *renderPass_depthC_image32fC;
	IMPL(nullptr) RenderPass *renderPass_window;
	IMPL(nullptr) RenderPass *renderPass_windowC;

	IMPL() std::shared_ptr<Framebuffer> pickUpFb;

	unsigned int pickUp(int x, int y, const std::function<void(CommandBuffer*)> &drawCallback);

	void processCmdLine(const std::string &str, bool record = true);

	enum WindowStyle
	{
		WindowStyleFrame = 1 << 0,
		WindowStyleResize = 1 << 1,
		WindowStyleFullscreen = 1 << 2
	};

	// must call in main thread
	int init(bool vulkan_debug, const std::string &path, int rcx, int rcy, int _window_cx, int _window_cy, const std::string &title, unsigned int _window_style, bool _only_2d = false);

	IMPL() HWND hWnd;
	IMPL() int window_cx;
	IMPL() int window_cy;
	IMPL() KeyState mouseLeft;
	IMPL() KeyState mouseMiddle;
	IMPL() KeyState mouseRight;
	IMPL() int mouseX; 
	IMPL() int mouseY;
	IMPL() int mousePrevX;
	IMPL() int mousePrevY;
	IMPL() int mouseDispX;
	IMPL() int mouseDispY;
	IMPL() int mouseScroll;
	IMPL() KeyState keyStates[256];
	IMPL() VkSurfaceKHR window_surface;
	IMPL() VkSwapchainKHR swapchain;
	IMPL() Image *window_images[2];
	IMPL() std::shared_ptr<Framebuffer> window_framebuffers[2];
	IMPL() VkSemaphore window_imageAvailable;
	IMPL() uint32_t window_imageIndex;
	IMPL() VkFence frameDone;
	IMPL() uint32_t FPS;
	IMPL(nullptr) PF_EVENT1 onKeyDown;
	IMPL(nullptr) PF_EVENT1 onKeyUp;
	IMPL(nullptr) PF_EVENT1 onChar;
	IMPL(nullptr) PF_EVENT2 onMouseLeftDown;
	IMPL(nullptr) PF_EVENT2 onMouseLeftUp;
	IMPL(nullptr) PF_EVENT2 onMouseMiddleDown;
	IMPL(nullptr) PF_EVENT2 onMouseMiddleUp;
	IMPL(nullptr) PF_EVENT2 onMouseRightDown;
	IMPL(nullptr) PF_EVENT2 onMouseRightUp;
	IMPL(nullptr) PF_EVENT2 onMouseMove;
	IMPL(nullptr) PF_EVENT1 onMouseWheel;
	IMPL(nullptr) PF_EVENT0 onRender;
	IMPL(nullptr) PF_EVENT0 onDestroy;
	IMPL() std::uint32_t window_style;
	IMPL(false) bool only_2d;

	std::pair<glm::ivec2, unsigned int> getWin32WndProp();

	enum EventType
	{
		EventTypeEvent,
		EventTypeOnlyOne
	};
	void addBeforeFrameEvent(const std::function<void()>&, int id = -1, EventType event_type = EventTypeEvent);

	// must call in main thread
	void beginFrame(bool clearBackground);

	void addCb(VkCommandBuffer cb);

	// must call in main thread
	void endFrame();

	void run();
}
