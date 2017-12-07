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
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data) = 0;
		void render(FrameCommandBufferList *cb_list, Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data);

	};

	struct PlainRenderer : Renderer
	{
		static bool first;

		struct DrawData
		{
			glm::mat4 mat;
			Model *model;
			UniformBuffer *bone_buffer;
			glm::vec4 color;
		};

		PlainRenderer();
		// count: H - mode(0: just color, 1: color with a front light, 2: just texture), L - count
		// user_data: pointer of DrawData
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data) override;
		static void render_to(CommandBuffer *cb, int mode, Camera *camera, int count, DrawData *data);
	};

	struct Object;
	struct WireframeRenderer : Renderer
	{
		static bool first;

		Object *last_obj = nullptr;
		std::unique_ptr<DescriptorSet> ds_anim;

		WireframeRenderer();
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data) override;
	};

	struct LinesRenderer : Renderer
	{
		static bool first;

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
		};

		LinesRenderer();
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data) override;
	};

	struct DeferredRenderer : Renderer
	{
		DeferredRenderer();
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data) override;
	};
}
