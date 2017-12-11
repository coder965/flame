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
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, void *user_data) = 0;
		void render(FrameCommandBufferList *cb_list, Framebuffer *framebuffer, bool clear, Camera *camera, void *user_data);

	};

	struct PlainRenderer : Renderer
	{
		static bool first;

		static UniformBuffer *last_bone_buffer;

		enum Mode
		{
			mode_just_color,
			mode_color_and_front_light,
			mode_just_texture,
			mode_wireframe
		};

		struct DrawData
		{
			Mode mode;
			VertexBuffer *vbuffer0 = nullptr;
			VertexBuffer *vbuffer1 = nullptr;
			IndexBuffer *ibuffer = nullptr;

			struct ObjData
			{
				glm::mat4 mat;
				struct GeoData
				{
					int index_count;
					int first_index = 0;
					int vertex_offset = 0;
					int instance_count = 1;
					int first_instance = 0;
				};
				std::vector<GeoData> geo_data;
				UniformBuffer *bone_buffer = nullptr;
				glm::vec4 color;

				void fill_with_model(Model *m);
				void fill_with_model_texture_mode(Model *m);
			};

			std::vector<ObjData> obj_data;
		};

		PlainRenderer();
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, void *user_data) override;
		void render_to(CommandBuffer *cb, Camera *camera, DrawData *data);
	};

	struct LinesRenderer : Renderer
	{
		static bool first;

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
		};

		struct DrawData
		{
			OnceVertexBuffer *vertex_buffer;
			int vertex_count;
		};

		LinesRenderer();
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, void *user_data) override;
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

		int staticIndirectCount = 0;
		int animatedIndirectCount = 0;

		DeferredRenderer(bool _enable_shadow, Image *dst);
		virtual void do_render(Framebuffer *framebuffer, bool clear, Camera *camera, void *user_data) override;
	};
}
