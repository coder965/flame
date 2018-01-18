#pragma once

#include <memory>

#include "../global.h"
#include "../math/math.h"
#include "../object.h"
#include "../resource/resource.h"
#include "graphics.h"
#include "renderer.h"

namespace tke
{
	struct CameraComponent;
	struct Model;
	struct UniformBuffer;
	struct ImmediateVertexBuffer;
	struct IndirectIndexBuffer;
	struct VertexBuffer;
	struct IndexBuffer;
	struct DescriptorSet;
	struct Framebuffer;
	struct CommandBuffer;
	struct Scene;

	struct Renderer : Object
	{

	};

	struct PlainRenderer
	{
		static bool first;

		std::unique_ptr<CommandBuffer> cb;

		static UniformBuffer *last_bone_buffer_mode0;
		static UniformBuffer *last_bone_buffer_mode2;
		static UniformBuffer *last_bone_buffer_mode3;

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
		void render(Framebuffer *framebuffer, bool clear, CameraComponent *camera, DrawData *data);
		void do_render(CommandBuffer *cb, CameraComponent *camera, DrawData *data);
		void add_to_drawlist();
	};

	struct LinesRenderer
	{
		static bool first;

		std::unique_ptr<CommandBuffer> cb;

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
		};

		struct DrawData
		{
			ImmediateVertexBuffer *vertex_buffer;
			int vertex_count;
		};

		LinesRenderer();
		void render(Framebuffer *framebuffer, bool clear, CameraComponent *camera, DrawData *data);
		void add_to_drawlist();
	};

	enum { MaxLightCount = 256 };
	enum { MaxStaticModelInstanceCount = 1024 };
	enum { MaxAnimatedModelInstanceCount = 8 };
	enum { MaxTerrainCount = 8 };
	enum { MaxWaterCount = 8 };

	enum { MaxShadowCount = 4 };
	enum { EnvrSizeCx = 128 * 4 };
	enum { EnvrSizeCy = 128 * 2 };
	enum { ShadowMapCx = 2048 };
	enum { ShadowMapCy = 2048 };

	struct DeferredRenderer : Renderer
	{
		static bool defe_inited;
		static bool shad_inited;

		bool sky_dirty;
		bool ambient_dirty;
		bool light_count_dirty;
		bool static_model_instance_count_dirty;
		bool animated_model_instance_count_dirty;
		bool terrain_count_dirty;
		bool water_count_dirty;

		int static_indirect_count;
		int animated_indirect_count;

		std::unique_ptr<CommandBuffer> cb_defe;
		std::unique_ptr<CommandBuffer> cb_shad;

		std::unique_ptr<UniformBuffer> matrixBuffer;
		std::unique_ptr<UniformBuffer> staticModelInstanceMatrixBuffer;
		std::unique_ptr<UniformBuffer> animatedModelInstanceMatrixBuffer;
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

		virtual bool on_message(Object *sender, Message msg) override;

		DeferredRenderer(bool _enable_shadow, Image *dst);
		void render(Scene *scene, CameraComponent *camera);
		void add_to_drawlist();
	};
}
