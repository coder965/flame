#pragma once

#include <memory>

#include "../core.h"
#include "../entity/camera.h"
#include "vulkan.h"
#include "command_buffer.h"
#include "framebuffer.h"

namespace tke
{
	struct Renderer
	{
		std::unique_ptr<CommandBuffer> cb;
		VkEvent renderFinished;

		Renderer();
		virtual ~Renderer();
		virtual void render(Framebuffer *framebuffer, Camera *camera, FrameCommandBufferList *cb_list, void *user_data) = 0;

	};

	struct PlainRenderer : Renderer
	{
		PlainRenderer();
		virtual void render(Framebuffer *framebuffer, Camera *camera, FrameCommandBufferList *cb_list, void *user_data) override;
	};

	struct Object;
	struct WireframeRenderer : Renderer
	{
		static bool first;

		Object *last_obj = nullptr;
		std::unique_ptr<DescriptorSet> ds_anim;

		WireframeRenderer();
		virtual void render(Framebuffer *framebuffer, Camera *camera, FrameCommandBufferList *cb_list, void *user_data) override;
	};

	struct DeferredRenderer : Renderer
	{
		DeferredRenderer();
		virtual void render(Framebuffer *framebuffer, Camera *camera, FrameCommandBufferList *cb_list, void *user_data) override;
	};
}
