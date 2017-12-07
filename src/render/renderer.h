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

		static UniformBuffer *last_bone_buffer;

		struct DrawData
		{
			glm::mat4 mat;
			Model *model;
			UniformBuffer *bone_buffer;
			glm::vec4 color;
		};

		PlainRenderer();
		// count: H - mode(0: just color, 1: color with a front light, 2: just texture, 3: wireframe), L - count
		// user_data: pointer of DrawData
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data) override;
		static void render_to(CommandBuffer *cb, int mode, Camera *camera, int count, DrawData *data);
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

	enum { MaxStaticObjectCount = 1024 };
	enum { MaxAnimatedObjectCount = 8 };
	enum { MaxLightCount = 256 };
	enum { MaxWaterCount = 8 };
	enum { MaxShadowCount = 4 };

	enum { EnvrSizeCx = 128 * 4 };
	enum { EnvrSizeCy = 128 * 2 };
	enum { ShadowMapCx = 2048 };
	enum { ShadowMapCy = 2048 };

	enum { MaxIndirectCount = 1024 };

	struct Object;
	struct DeferredRenderer : Renderer
	{
		static bool defe_inited;
		static bool shad_inited;

		std::unique_ptr<UniformBuffer> matrixBuffer;
		std::unique_ptr<UniformBuffer> staticObjectMatrixBuffer;
		std::unique_ptr<UniformBuffer> animatedObjectMatrixBuffer;
		std::unique_ptr<UniformBuffer> terrainBuffer;
		std::unique_ptr<UniformBuffer> waterBuffer;
		std::unique_ptr<UniformBuffer> lightBuffer;
		std::unique_ptr<UniformBuffer> ambientBuffer;
		std::unique_ptr<IndirectIndexBuffer> staticObjectIndirectBuffer;
		std::unique_ptr<IndirectIndexBuffer> animatedObjectIndirectBuffer;
		std::unique_ptr<Image> envrImage;
		std::unique_ptr<Image> mainImage;
		std::unique_ptr<Image> depthImage;
		std::unique_ptr<Image> albedoAlphaImage;
		std::unique_ptr<Image> normalHeightImage;
		std::unique_ptr<Image> specRoughnessImage;
		std::unique_ptr<DescriptorSet> ds_mrt;
		std::unique_ptr<DescriptorSet> ds_mrtAnim;
		std::unique_ptr<DescriptorSet> ds_mrtAnim_bone;
		std::unique_ptr<DescriptorSet> ds_terrain;
		std::unique_ptr<DescriptorSet> ds_water;
		std::unique_ptr<DescriptorSet> ds_defe;
		std::unique_ptr<DescriptorSet> ds_comp;

		bool enable_shadow;
		std::unique_ptr<UniformBuffer> shadowBuffer;
		std::unique_ptr<Image> esmImage;
		std::unique_ptr<Image> esmDepthImage;
		std::shared_ptr<Framebuffer> fb_esm[MaxShadowCount * 6];
		std::unique_ptr<DescriptorSet> ds_esm;
		std::unique_ptr<DescriptorSet> ds_esmAnim;

		Resource resource;

		std::shared_ptr<Framebuffer> framebuffer;

		std::vector<Object*> staticObjects;
		std::vector<Object*> animatedObjects;
		int staticIndirectCount = 0;
		int animatedIndirectCount = 0;
		std::vector<Light*> shadowLights;

		DeferredRenderer(bool _enable_shadow, Image *dst);
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data) override;
		void update(Scene *scene);
	};
}
