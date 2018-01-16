#include "../global.h"
#include "../spare_list.h"
#include "../model/model.h"
#include "../entity/scene.h"
#include "synchronization.h"
#include "buffer.h"
#include "image.h"
#include "material.h"
#include "renderpass.h"
#include "framebuffer.h"
#include "descriptor.h"
#include "pipeline.h"
#include "sampler.h"
#include "command_buffer.h"
#include "renderer.h"
#include "../model/animation.h"
#include "../entity/camera.h"
#include "../entity/light.h"
#include "../entity/model_instance.h"
#include "../entity/terrain.h"
#include "../entity/water.h"
#include "../engine.h"

namespace tke
{
	void PlainRenderer::DrawData::ObjData::fill_with_model(Model *m)
	{
		geo_data.resize(1);
		geo_data[0].index_count = m->indice_count;
		geo_data[0].first_index = m->indiceBase;
		geo_data[0].vertex_offset = m->vertexBase;
		geo_data[0].instance_count = 1;
		geo_data[0].first_instance = 0;
	}

	void PlainRenderer::DrawData::ObjData::fill_with_model_texture_mode(Model *m)
	{
		for (int i = 0; i < m->geometries.size(); i++)
		{
			auto &g = m->geometries[i];
			if (g->material->albedoAlphaMap)
			{
				GeoData data;
				auto &g = m->geometries[i];
				data.index_count = g->indiceCount;
				data.first_index = m->indiceBase + g->indiceBase;
				data.vertex_offset = m->vertexBase;
				data.instance_count = 1;
				data.first_instance = g->material->albedoAlphaMap->material_index;
				geo_data.push_back(data);
			}
		}
	}

	static Pipeline *pipeline_plain;
	static Pipeline *pipeline_plain_anim;
	static Pipeline *pipeline_frontlight;
	static Pipeline *pipeline_material;
	static Pipeline *pipeline_material_anim;
	static Pipeline *pipeline_wireframe;
	static Pipeline *pipeline_wireframe_anim;
	bool PlainRenderer::first = true;
	UniformBuffer *PlainRenderer::last_bone_buffer_mode0;
	UniformBuffer *PlainRenderer::last_bone_buffer_mode2;
	UniformBuffer *PlainRenderer::last_bone_buffer_mode3;
	PlainRenderer::PlainRenderer()
	{
		if (first)
		{
			pipeline_plain = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0}, { TokenF32V2, 0}, { TokenF32V3, 0}, { TokenF32V3, 0} })
				.depth_test(true)
				.depth_write(true)
				.add_shader(engine_path + "shader/plain3d/plain3d.vert", {})
				.add_shader(engine_path + "shader/plain3d/plain3d.frag", {}),
				renderPass_depthC_image8, 0);
			pipeline_plain_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 },{ TokenF32V4, 1 },{ TokenF32V4, 1 } })
				.depth_test(true)
				.depth_write(true)
				.add_shader(engine_path + "shader/plain3d/plain3d.vert", { "ANIM" })
				.add_shader(engine_path + "shader/plain3d/plain3d.frag", { "ANIM" }),
				renderPass_depthC_image8, 0, true);
			pipeline_frontlight = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 } })
				.depth_test(true)
				.depth_write(true)
				.add_shader(engine_path + "shader/plain3d/plain3d.vert", { "USE_NORMAL" })
				.add_shader(engine_path + "shader/plain3d/plain3d.frag", { "USE_NORMAL" }),
				renderPass_depthC_image8, 0);
			pipeline_material = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 } })
				.depth_test(true)
				.depth_write(true)
				.add_shader(engine_path + "shader/plain3d/plain3d.vert", { "USE_MATERIAL" })
				.add_shader(engine_path + "shader/plain3d/plain3d.frag", { "USE_MATERIAL" }),
				renderPass_depthC_image8, 0);
			pipeline_material_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 },{ TokenF32V4, 1 },{ TokenF32V4, 1 } })
				.depth_test(true)
				.depth_write(true)
				.add_shader(engine_path + "shader/plain3d/plain3d.vert", { "ANIM", "USE_MATERIAL" })
				.add_shader(engine_path + "shader/plain3d/plain3d.frag", { "ANIM", "USE_MATERIAL" }),
				renderPass_depthC_image8, 0, true);
			pipeline_wireframe = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 } })
				.polygon_mode(VK_POLYGON_MODE_LINE)
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/plain3d/plain3d.vert", {})
				.add_shader(engine_path + "shader/plain3d/plain3d.frag", {}),
				renderPass_image8, 0);
			pipeline_wireframe_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 },{ TokenF32V4, 1 },{ TokenF32V4, 1 } })
				.polygon_mode(VK_POLYGON_MODE_LINE)
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/plain3d/plain3d.vert", { "ANIM" })
				.add_shader(engine_path + "shader/plain3d/plain3d.frag", { "ANIM" }),
				renderPass_image8, 0, true);

			first = false;
		}

		cb = std::make_unique<CommandBuffer>();
	}

	void PlainRenderer::render(Framebuffer *framebuffer, bool clear, CameraComponent *camera, DrawData *data)
	{
		cb->reset();
		cb->begin();
		RenderPass *rp;
		if (data->mode == mode_wireframe)
		{
			if (clear)
				rp = renderPass_image8C;
			else
				rp = renderPass_image8;
		}
		else
		{
			if (clear)
				rp = renderPass_depthC_image8C;
			else
				rp = renderPass_depthC_image8;
		}
		cb->beginRenderPass(rp, framebuffer);
		do_render(cb.get(), camera, data);
		cb->endRenderPass();
		cb->end();
	}

	void PlainRenderer::do_render(CommandBuffer *cb, CameraComponent *camera, DrawData *data)
	{
		if (data->vbuffer0)
		{
			if (data->vbuffer1)
				cb->bindVertexBuffer2(data->vbuffer0, data->vbuffer1);
			else
				cb->bindVertexBuffer(data->vbuffer0);
			cb->bindIndexBuffer(data->ibuffer);
		}
		else
		{
			cb->bindVertexBuffer2(vertexStatBuffer.get(), vertexAnimBuffer.get());
			cb->bindIndexBuffer(indexBuffer.get());
		}

		struct
		{
			glm::mat4 modelview;
			glm::mat4 proj;
			glm::vec4 color;
		}pc;
		pc.proj = camera->get_proj_matrix();

		for (int i = 0; i < data->obj_data.size(); i++)
		{
			auto &d = data->obj_data[i];

			switch (data->mode)
			{
				case mode_just_color:
					if (!d.bone_buffer)
						cb->bindPipeline(pipeline_plain);
					else
					{
						cb->bindPipeline(pipeline_plain_anim);
						if (last_bone_buffer_mode0 != d.bone_buffer)
						{
							updateDescriptorSets(1, &pipeline_plain_anim->descriptorSet->bufferWrite(0, 0, d.bone_buffer));
							last_bone_buffer_mode0 = d.bone_buffer;
						}
						cb->bindDescriptorSet();
					}
					break;
				case mode_color_and_front_light:
					cb->bindPipeline(pipeline_frontlight);
					break;
				case mode_just_texture:
				{
					if (!d.bone_buffer)
					{
						cb->bindPipeline(pipeline_material);
						cb->bindDescriptorSet(&ds_material->v, 1, 1);
					}
					else
					{
						cb->bindPipeline(pipeline_material_anim);
						if (last_bone_buffer_mode2 != d.bone_buffer)
						{
							updateDescriptorSets(1, &pipeline_material_anim->descriptorSet->bufferWrite(0, 0, d.bone_buffer));
							last_bone_buffer_mode2 = d.bone_buffer;
						}
						VkDescriptorSet sets[] = {
							pipeline_material->descriptorSet->v,
							ds_material->v
						};
						cb->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
					}
					break;
				}
				case mode_wireframe:
					if (!d.bone_buffer)
						cb->bindPipeline(pipeline_wireframe);
					else
					{
						cb->bindPipeline(pipeline_wireframe_anim);
						if (last_bone_buffer_mode3 != d.bone_buffer)
						{
							updateDescriptorSets(1, &pipeline_wireframe_anim->descriptorSet->bufferWrite(0, 0, d.bone_buffer));
							last_bone_buffer_mode3 = d.bone_buffer;
						}
						cb->bindDescriptorSet();
					}
					break;
			}

			pc.modelview = camera->get_view_matrix() * d.mat;
			pc.color = d.color;
			cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
			for (auto &d : d.geo_data)
				cb->drawIndex(d.index_count, d.first_index, d.vertex_offset, d.instance_count, d.first_instance);
		}
	}

	void PlainRenderer::add_to_drawlist()
	{
		tke::add_to_drawlist(cb->v);
	}

	static Pipeline *pipeline_lines;
	bool LinesRenderer::first = true;
	LinesRenderer::LinesRenderer()
	{
		if (first)
		{
			pipeline_lines = new Pipeline(PipelineCreateInfo()
					.cx(-1).cy(-1)
					.vertex_input_state({ { TokenF32V3, 0}, { TokenF32V3, 0} })
					.primitive_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
					.polygon_mode(VK_POLYGON_MODE_LINE)
					.cull_mode(VK_CULL_MODE_NONE)
					.add_shader(engine_path + "shader/plain3d/plain3d_line.vert", {})
					.add_shader(engine_path + "shader/plain3d/plain3d_line.frag", {}),
					renderPass_image8, 0);

			first = false;
		}

		cb = std::make_unique<CommandBuffer>();
	}

	void LinesRenderer::render(Framebuffer *framebuffer, bool clear, CameraComponent *camera, DrawData *data)
	{
		cb->reset();
		cb->begin();

		cb->beginRenderPass(renderPass_image8, framebuffer);

		cb->bindVertexBuffer(data->vertex_buffer);
		cb->bindPipeline(pipeline_lines);

		glm::mat4 mvp = camera->get_proj_matrix() * camera->get_view_matrix();
		cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);
		cb->draw(data->vertex_count);

		cb->endRenderPass();

		cb->end();
	}

	void LinesRenderer::add_to_drawlist()
	{
		tke::add_to_drawlist(cb->v);
	}

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

	struct AmbientBufferShaderStruct
	{
		glm::vec3 color;
		glm::uint envr_max_mipmap;
		glm::vec4 fogcolor;
	};

	struct LightShaderStruct
	{
		glm::vec4 coord;    // xyz - coord(point/spot)/dir(parallax), w - the light type
		glm::vec4 color;    // rgb - color, a - shadow index(-1 is no shadow)
		glm::vec4 spotData; // xyz - spot direction, a - spot range
	};

	struct LightBufferShaderStruct
	{
		unsigned int count;
		unsigned int dummy0;
		unsigned int dummy1;
		unsigned int dummy2;

		LightShaderStruct lights[MaxLightCount];
	};

	struct TerrainShaderStruct
	{
		glm::vec3 coord;
		int block_cx;
		int block_cy;
		float block_size;
		float terrain_height;
		float displacement_height;
		float tessellation_factor;
		float tiling_scale;
		unsigned int material_count;
		union
		{
			struct
			{
				unsigned char x;
				unsigned char y;
				unsigned char z;
				unsigned char w;
			};
			unsigned char v[4];
			unsigned int packed;
		}material_index;
	};

	struct WaterShaderStruct
	{
		glm::vec3 coord;
		int block_cx;
		int block_cy;
		float block_size;
		float height;
		float tessellation_factor;
		float tiling_scale;
		float mapDimension;
		unsigned int dummy0;
		unsigned int dummy1;
	};

	static SpareList lights(MaxLightCount);
	static SpareList static_model_instances(MaxStaticModelInstanceCount);
	static SpareList animated_model_instances(MaxAnimatedModelInstanceCount);
	static SpareList terrains(MaxTerrainCount);
	static SpareList waters(MaxWaterCount);
	static SpareList shadow_lights(MaxShadowCount);

	VkPipelineVertexInputStateCreateInfo vertexStatInputState;
	VkPipelineVertexInputStateCreateInfo vertexAnimInputState;
	VkPipelineVertexInputStateCreateInfo terrian_vertex_input_state;

	static Pipeline *scattering_pipeline;
	static Pipeline *output_debug_panorama_pipeline;
	static Pipeline *downsample_pipeline;
	static Pipeline *convolve_pipeline;
	static Pipeline *copy_pipeline;
	static Pipeline *mrt_pipeline;
	static Pipeline *mrt_anim_pipeline;
	static Pipeline *terrain_pipeline;
	static Pipeline *water_pipeline;
	static Pipeline *deferred_pipeline;
	static Pipeline *compose_pipeline;
	static Pipeline *esm_pipeline;
	static Pipeline *esm_anim_pipeline;
	static RenderPass *defe_renderpass;
	static Image *envr_image_downsample[3] = {};
	bool DeferredRenderer::defe_inited = false;
	bool DeferredRenderer::shad_inited = false;

	bool DeferredRenderer::on_message(Object *sender, Message msg)
	{
		switch (msg)
		{
			case MessageSkyDirty:
				sky_dirty = true;
				return true;
			case MessageAmbientDirty:
				ambient_dirty = true;
				return true;
			case MessageComponentAdd:
			{
				auto c = (Component*)sender;
				switch (c->get_type())
				{
					case ComponentTypeLight:
					{
						auto l = (LightComponent*)c;
						auto index = lights.add(l);
						if (index != -2)
						{
							l->set_light_index(index);
							light_count_dirty = true;
							if (l->is_enable_shadow())
							{
								auto index = shadow_lights.add(l);
								if (index != -2)
									l->set_shadow_index(index);
							}
						}
						break;
					}
					case ComponentTypeModelInstance:
					{
						auto i = (ModelInstanceComponent*)c;
						auto index = i->get_model()->vertex_skeleton ? 
							animated_model_instances.add(i) :
							static_model_instances.add(i);
						if (index != -2)
						{
							i->set_instance_index(index);
							if (i->get_model()->vertex_skeleton)
								animated_model_instance_count_dirty = true;
							else
								static_model_instance_count_dirty = true;
						}
						break;
					}
					case ComponentTypeTerrain:
					{
						auto t = (TerrainComponent*)c;
						auto index = terrains.add(t);
						if (index != -2)
						{
							t->set_terrain_index(index);
							terrain_count_dirty = true;
						}
						break;
					}
					case ComponentTypeWater:
					{
						auto w = (WaterComponent*)c;
						auto index = waters.add(w);
						if (index != -2)
						{
							w->set_water_index(index);
							water_count_dirty = true;
						}
						break;
					}
				}
				return true;
			}
			case MessageComponentRemove:
			{
				auto c = (Component*)sender;
				switch (c->get_type())
				{
					case ComponentTypeLight:
					{
						auto l = (LightComponent*)c;
						auto index = l->get_light_index();
						if (index != -1)
						{
							lights.remove(l);
							light_count_dirty = true;
							if (l->is_enable_shadow() && l->get_shadow_index() != -1)
								shadow_lights.remove(l);
						}
						break;
					}
					case ComponentTypeModelInstance:
					{
						auto i = (ModelInstanceComponent*)c;
						auto index = i->get_instance_index();
						if (index != -1)
						{
							if (i->get_model()->vertex_skeleton)
								animated_model_instances.remove(i);
							else
								static_model_instances.remove(i);
							if (i->get_model()->vertex_skeleton)
								animated_model_instance_count_dirty = true;
							else
								static_model_instance_count_dirty = true;
						}
						break;
					}
					case ComponentTypeTerrain:
					{
						auto t = (TerrainComponent*)c;
						auto index = t->get_terrain_index();
						if (index != -1)
						{
							terrains.remove(t);
							terrain_count_dirty = true;
						}
						break;
					}
					case ComponentTypeWater:
					{
						auto w = (WaterComponent*)c;
						auto index = w->get_water_index();
						if (index != -1)
						{
							waters.remove(w);
							water_count_dirty = true;
						}
						break;
					}
				}
				return true;
			}
			case MessageToggleShaodw:
			{
				auto l = (LightComponent*)sender;
				if (l->is_enable_shadow())
				{
					auto index = shadow_lights.add(l);
					if (index != -2)
						l->set_shadow_index(index);
				}
				else
				{
					if (l->get_shadow_index() != -1)
						shadow_lights.remove(l);
				}
				return true;
			}
		}
		return false;
	}

	DeferredRenderer::DeferredRenderer(bool _enable_shadow, Image *dst)
		:enable_shadow(_enable_shadow), resource(&globalResource)
	{
		sky_dirty = true;
		ambient_dirty = true;
		light_count_dirty = true;
		static_model_instance_count_dirty = true;
		animated_model_instance_count_dirty = true;
		terrain_count_dirty = true;

		if (!defe_inited)
		{
			VkAttachmentDescription atts[] = {
				colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE), // main
				depthAttachmentDesc(VK_FORMAT_D32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR),				 // depth
				colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // albedo alpha
				colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // normal height
				colorAttachmentDesc(VK_FORMAT_R16G16B16A16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR),		 // spec roughness
				colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_DONT_CARE)		 // dst
			};
			VkAttachmentReference main_col_ref = { 0, VK_IMAGE_LAYOUT_GENERAL };
			VkAttachmentReference mrt_col_ref[] = {
				{2, VK_IMAGE_LAYOUT_GENERAL},
			{3, VK_IMAGE_LAYOUT_GENERAL},
			{4, VK_IMAGE_LAYOUT_GENERAL}
			};
			VkAttachmentReference dep_ref = { 1, VK_IMAGE_LAYOUT_GENERAL };
			VkAttachmentReference dst_col_ref = { 5, VK_IMAGE_LAYOUT_GENERAL };
			VkSubpassDescription subpasses[] = {
				subpassDesc(ARRAYSIZE(mrt_col_ref), mrt_col_ref, &dep_ref), // mrt
				subpassDesc(1, &main_col_ref),                              // deferred
				subpassDesc(1, &dst_col_ref)                                // compose
			};

			VkSubpassDependency dependencies[] = {
				subpassDependency(0, 1),
				subpassDependency(1, 2)
			};

			defe_renderpass = new RenderPass(ARRAYSIZE(atts), atts, ARRAYSIZE(subpasses), subpasses, ARRAYSIZE(dependencies), dependencies);

			scattering_pipeline = new Pipeline(PipelineCreateInfo()
				.cx(512).cy(256)
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/fullscreen.vert", { "USE_UV" })
				.add_shader(engine_path + "shader/sky/scattering.frag", {}),
				renderPass_image16, 0);
			output_debug_panorama_pipeline = new Pipeline(PipelineCreateInfo()
				.cx(512).cy(256)
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/fullscreen.vert", { "USE_UV" })
				.add_shader(engine_path + "shader/sky/output_debug_panorama.frag", {}),
				renderPass_image8, 0);
			downsample_pipeline = new Pipeline(PipelineCreateInfo()
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/fullscreen.vert", { "USE_UV" })
				.add_shader(engine_path + "shader/sky/downsample.frag", {})
				, renderPass_image16, 0, true);
			convolve_pipeline = new Pipeline(PipelineCreateInfo()
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/fullscreen.vert", { "USE_UV" })
				.add_shader(engine_path + "shader/sky/convolve.frag", {}),
				renderPass_image16, 0, true);
			copy_pipeline = new Pipeline(PipelineCreateInfo()
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/fullscreen.vert", { "USE_UV" })
				.add_shader(engine_path + "shader/copy.frag", {}),
				renderPass_image16, 0, true);
			mrt_pipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 } })
				.depth_test(true)
				.depth_write(true)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_shader(engine_path + "shader/deferred/mrt.vert", {})
				.add_shader(engine_path + "shader/deferred/mrt.frag", {})
				.add_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_link("ubo_object_static_", "StaticObjectMatrix.UniformBuffer"),
				defe_renderpass, 0);
			mrt_anim_pipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 },{ TokenF32V4, 1 },{ TokenF32V4, 1 } })
				.depth_test(true)
				.depth_write(true)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_shader(engine_path + "shader/deferred/mrt.vert", { "ANIM" })
				.add_shader(engine_path + "shader/deferred/mrt.frag", { "ANIM" })
				.add_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_link("ubo_object_animated_", "AnimatedObjectMatrix.UniformBuffer"),
				defe_renderpass, 0);
			terrain_pipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 } })
				.patch_control_points(4)
				.depth_test(true)
				.depth_write(true)
				.primitive_topology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_shader(engine_path + "shader/deferred/terrain.vert", {})
				.add_shader(engine_path + "shader/deferred/terrain.tesc", {})
				.add_shader(engine_path + "shader/deferred/terrain.tese", {})
				.add_shader(engine_path + "shader/deferred/terrain.frag", {})
				.add_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_link("ubo_terrain_", "Terrain.UniformBuffer"),
				defe_renderpass, 0);
			water_pipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.patch_control_points(4)
				.depth_test(true)
				.depth_write(true)
				.primitive_topology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_shader(engine_path + "shader/deferred/water.vert", {})
				.add_shader(engine_path + "shader/deferred/water.tesc", {})
				.add_shader(engine_path + "shader/deferred/water.tese", {})
				.add_shader(engine_path + "shader/deferred/water.frag", {})
				.add_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_link("ubo_water_", "Water.UniformBuffer"),
				defe_renderpass, 0);
			deferred_pipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/fullscreen.vert", { "USE_VIEW" })
				.add_shader(engine_path + "shader/deferred/deferred.frag", { "USE_PBR", "USE_IBL" })
				.add_link("ubo_constant_", "Constant.UniformBuffer")
				.add_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_link("img_depth", "Depth.Image", 0, plainUnnormalizedSampler)
				.add_link("img_albedo_alpha", "AlbedoAlpha.Image", 0, plainUnnormalizedSampler)
				.add_link("img_normal_height", "NormalHeight.Image", 0, plainUnnormalizedSampler)
				.add_link("img_spec_roughness", "SpecRoughness.Image", 0, plainUnnormalizedSampler)
				.add_link("ubo_light_", "Light.UniformBuffer")
				.add_link("img_envr", "Envr.Image", 0, colorSampler)
				.add_link("ubo_ambient_", "Ambient.UniformBuffer")
				.add_link("img_shadow", "Shadow.Image")
				.add_link("ubo_shadow_", "Shadow.UniformBuffer"),
				defe_renderpass, 1);
			compose_pipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.cull_mode(VK_CULL_MODE_NONE)
				.add_shader(engine_path + "shader/fullscreen.vert", {})
				.add_shader(engine_path + "shader/compose/compose.frag", {})
				.add_link("img_source", "Main.Image", 0, plainUnnormalizedSampler),
				defe_renderpass, 2);

			defe_inited = true;
		}

		cb_defe = std::make_unique<CommandBuffer>();

		matrixBuffer = std::make_unique<UniformBuffer>(sizeof MatrixBufferShaderStruct);
		staticModelInstanceMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxStaticModelInstanceCount);
		animatedModelInstanceMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxAnimatedModelInstanceCount);
		terrainBuffer = std::make_unique<UniformBuffer>(sizeof(TerrainShaderStruct) * MaxTerrainCount);
		waterBuffer = std::make_unique<UniformBuffer>(sizeof(WaterShaderStruct) * MaxWaterCount);
		lightBuffer = std::make_unique<UniformBuffer>(sizeof(LightBufferShaderStruct));
		ambientBuffer = std::make_unique<UniformBuffer>(sizeof AmbientBufferShaderStruct);
		staticObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * MaxStaticModelInstanceCount);
		animatedObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * MaxAnimatedModelInstanceCount);

		envrImage = std::make_unique<Image>(EnvrSizeCx, EnvrSizeCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 4);
		for (int i = 0; i < 3; i++)
			envr_image_downsample[i] = new Image(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1),
				VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mainImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		depthImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		albedoAlphaImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		normalHeightImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		specRoughnessImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		ds_mrt = std::make_unique<DescriptorSet>(mrt_pipeline);
		ds_mrtAnim = std::make_unique<DescriptorSet>(mrt_anim_pipeline);
		ds_mrtAnim_bone = std::make_unique<DescriptorSet>(mrt_anim_pipeline, 2);
		ds_terrain = std::make_unique<DescriptorSet>(terrain_pipeline);
		ds_water = std::make_unique<DescriptorSet>(water_pipeline);
		ds_defe = std::make_unique<DescriptorSet>(deferred_pipeline);
		ds_comp = std::make_unique<DescriptorSet>(compose_pipeline);

		resource.setImage(envrImage.get(), "Envr.Image");
		resource.setImage(mainImage.get(), "Main.Image");
		resource.setImage(depthImage.get(), "Depth.Image");
		resource.setImage(albedoAlphaImage.get(), "AlbedoAlpha.Image");
		resource.setImage(normalHeightImage.get(), "NormalHeight.Image");
		resource.setImage(specRoughnessImage.get(), "SpecRoughness.Image");

		resource.setBuffer(matrixBuffer.get(), "Matrix.UniformBuffer");
		resource.setBuffer(staticModelInstanceMatrixBuffer.get(), "StaticObjectMatrix.UniformBuffer");
		resource.setBuffer(animatedModelInstanceMatrixBuffer.get(), "AnimatedObjectMatrix.UniformBuffer");
		resource.setBuffer(terrainBuffer.get(), "Terrain.UniformBuffer");
		resource.setBuffer(waterBuffer.get(), "Water.UniformBuffer");
		resource.setBuffer(lightBuffer.get(), "Light.UniformBuffer");
		resource.setBuffer(ambientBuffer.get(), "Ambient.UniformBuffer");
		resource.setBuffer(staticObjectIndirectBuffer.get(), "Scene.Static.IndirectBuffer");
		resource.setBuffer(animatedObjectIndirectBuffer.get(), "Scene.Animated.IndirectBuffer");

		if (enable_shadow)
		{
			if (!shad_inited)
			{
				esm_pipeline = new Pipeline(PipelineCreateInfo()
					.cx(2048).cy(2048)
					.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 } })
					.depth_test(true)
					.depth_write(true)
					.add_shader(engine_path + "shader/esm/esm.vert", {})
					.add_shader(engine_path + "shader/esm/esm.frag", {})
					.add_link("ubo_constant_", "Constant.UniformBuffer")
					.add_link("ubo_object_static_", "StaticObjectMatrix.UniformBuffer")
					.add_link("u_shadow_", "Shadow.UniformBuffer"),
					renderPass_depthC_image8C, 0);
				esm_anim_pipeline = new Pipeline(PipelineCreateInfo()
					.cx(2048).cy(2048)
					.vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 },{ TokenF32V4, 1 },{ TokenF32V4, 1 } })
					.depth_test(true)
					.depth_write(true)
					.add_shader(engine_path + "shader/esm/esm.vert", { "ANIM" })
					.add_shader(engine_path + "shader/esm/esm.frag", { "ANIM" })
					.add_link("ubo_constant_", "Constant.UniformBuffer")
					.add_link("ubo_object_animated_", "AnimatedObjectMatrix.UniformBuffer")
					.add_link("u_shadow_", "Shadow.UniformBuffer"),
					renderPass_depthC_image8C, 0);

				shad_inited = true;
			}

			cb_shad = std::make_unique<CommandBuffer>();

			shadowBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxShadowCount);

			esmImage = std::make_unique<Image>(ShadowMapCx, ShadowMapCy, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, MaxShadowCount * 6);
			esmDepthImage = std::make_unique<Image>(ShadowMapCx, ShadowMapCy, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			resource.setImage(esmImage.get(), "Shadow.Image");

			for (int i = 0; i < MaxShadowCount * 6; i++)
			{
				VkImageView views[] = {
					esmImage->getView(0, 1, i),
					esmDepthImage->getView()
				};
				fb_esm[i] = getFramebuffer(ShadowMapCx, ShadowMapCy, renderPass_depthC_image32fC, TK_ARRAYSIZE(views), views);
			}

			ds_esm = std::make_unique<DescriptorSet>(esm_pipeline);
			ds_esmAnim = std::make_unique<DescriptorSet>(esm_anim_pipeline);

			resource.setBuffer(shadowBuffer.get(), "Shadow.UniformBuffer");

			esm_pipeline->linkDescriptors(ds_esm.get(), &resource);
			esm_anim_pipeline->linkDescriptors(ds_esmAnim.get(), &resource);
		}

		mrt_pipeline->linkDescriptors(ds_mrt.get(), &resource);
		mrt_anim_pipeline->linkDescriptors(ds_mrtAnim.get(), &resource);
		terrain_pipeline->linkDescriptors(ds_terrain.get(), &resource);
		water_pipeline->linkDescriptors(ds_water.get(), &resource);
		deferred_pipeline->linkDescriptors(ds_defe.get(), &resource);
		compose_pipeline->linkDescriptors(ds_comp.get(), &resource);

		{
			VkImageView views[] = {
				mainImage->getView(),
				depthImage->getView(),
				albedoAlphaImage->getView(),
				normalHeightImage->getView(),
				specRoughnessImage->getView(),
				dst->getView(),
			};
			framebuffer = getFramebuffer(res_cx, res_cy, defe_renderpass, ARRAYSIZE(views), views);
		}
	}

	void DeferredRenderer::render(Scene *scene, CameraComponent *camera)
	{
		{ // always update the matrix buffer
			MatrixBufferShaderStruct stru;
			stru.proj = camera->get_proj_matrix();
			stru.projInv = camera->get_proj_matrix_inverse();
			stru.view = camera->get_view_matrix();
			stru.viewInv = camera->get_parent()->get_matrix();
			stru.projView = stru.proj * stru.view;
			stru.projViewRotate = stru.proj * glm::mat4(glm::mat3(stru.view));
			memcpy(stru.frustumPlanes, camera->get_frustum_planes(), sizeof(MatrixBufferShaderStruct::frustumPlanes));
			stru.viewportDim = glm::vec2(res_cx, res_cy);
			matrixBuffer->update(&stru, defalut_staging_buffer);
		}

		if (sky_dirty)
		{
			static const auto funUpdateIBL = [&]() {
				for (int i = 0; i < envrImage->levels.size() - 1; i++)
				{
					auto cb = begineOnceCommandBuffer();
					auto fb = getFramebuffer(envr_image_downsample[i], renderPass_image16);

					cb->beginRenderPass(renderPass_image16, fb.get());
					cb->bindPipeline(downsample_pipeline);
					cb->setViewportAndScissor(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1));
					auto size = glm::vec2(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1));
					cb->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof glm::vec2, &size);
					updateDescriptorSets(1, &downsample_pipeline->descriptorSet->imageWrite(0, 0, i == 0 ? envrImage.get() : envr_image_downsample[i - 1], plainSampler));
					cb->bindDescriptorSet();
					cb->draw(3);
					cb->endRenderPass();

					endOnceCommandBuffer(cb);
				}

				for (int i = 1; i < envrImage->levels.size(); i++)
				{
					auto cb = begineOnceCommandBuffer();
					auto fb = getFramebuffer(envrImage.get(), renderPass_image16, i);

					cb->beginRenderPass(renderPass_image16, fb.get());
					cb->bindPipeline(convolve_pipeline);
					auto data = 1.f + 1024.f - 1024.f * (i / 3.f);
					cb->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &data);
					cb->setViewportAndScissor(EnvrSizeCx >> i, EnvrSizeCy >> i);
					updateDescriptorSets(1, &convolve_pipeline->descriptorSet->imageWrite(0, 0, envr_image_downsample[i - 1], plainSampler));
					cb->bindDescriptorSet();
					cb->draw(3);
					cb->endRenderPass();

					endOnceCommandBuffer(cb);
				}
			};

			if (!scene->sky)
				envrImage->clear(glm::vec4(0.f));
			else
			{
				switch (scene->sky->type)
				{
					case SkyTypeDebug:
					{
						auto cb = begineOnceCommandBuffer();
						auto fb = getFramebuffer(envrImage.get(), renderPass_image8);

						cb->beginRenderPass(renderPass_image8, fb.get());
						cb->bindPipeline(output_debug_panorama_pipeline);
						cb->draw(3);
						cb->endRenderPass();

						endOnceCommandBuffer(cb);

						funUpdateIBL();

						break;
					}
					case SkyTypeAtmosphereScattering:
					{
						auto as = (SkyAtmosphereScattering*)scene->sky.get();

						auto cb = begineOnceCommandBuffer();
						auto fb = getFramebuffer(envrImage.get(), renderPass_image16);

						cb->beginRenderPass(renderPass_image16, fb.get());
						cb->bindPipeline(scattering_pipeline);
						auto euler = as->sun_light->get_parent()->get_euler();
						auto dir = glm::vec2(euler.x, euler.z);
						cb->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(dir), &dir);
						cb->draw(3);
						cb->endRenderPass();

						endOnceCommandBuffer(cb);

						funUpdateIBL();

						break;
					}
					case SkyTypePanorama:
					{
						auto pa = (SkyPanorama*)scene->sky.get();

						if (pa->panoImage)
						{
							auto cb = begineOnceCommandBuffer();
							auto fb = getFramebuffer(envrImage.get(), renderPass_image16);

							cb->beginRenderPass(renderPass_image16, fb.get());
							cb->bindPipeline(copy_pipeline);
							cb->setViewportAndScissor(EnvrSizeCx, EnvrSizeCy);
							updateDescriptorSets(1, &copy_pipeline->descriptorSet->imageWrite(0, 0, pa->panoImage.get(), colorSampler));
							cb->bindDescriptorSet();
							cb->draw(3);
							cb->endRenderPass();

							endOnceCommandBuffer(cb);

							funUpdateIBL();
						}
						else
							envrImage->clear(glm::vec4(0.f));
						break;
					}
				}
			}

			sky_dirty = false;
		}
		if (ambient_dirty)
		{
			AmbientBufferShaderStruct stru;
			stru.color = scene->ambientColor;
			stru.envr_max_mipmap = envrImage->levels.size() - 1;
			stru.fogcolor = glm::vec4(scene->fogColor, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
			ambientBuffer->update(&stru, defalut_staging_buffer);

			ambient_dirty = false;
		}
		static const auto fUpdateModelInstanceMatrixBuffer = [&](SpareList &list, Buffer *buffer) {
			if (list.get_size() > 0)
			{
				std::vector<VkBufferCopy> ranges;
				auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(glm::mat4) * list.get_size());

				list.iterate([&](int index, void *p, bool &remove) {
					auto i = (ModelInstanceComponent*)p;
					auto n = i->get_parent();
					if (n->is_transform_dirty())
					{
						auto srcOffset = sizeof(glm::mat4) * ranges.size();
						memcpy(map + srcOffset, &n->get_world_matrix(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * index;
						range.size = sizeof(glm::mat4);
						ranges.push_back(range);
					}
				});
				defalut_staging_buffer->unmap();
				defalut_staging_buffer->copyTo(buffer, ranges.size(), ranges.data());
			}
		};
		fUpdateModelInstanceMatrixBuffer(static_model_instances, staticModelInstanceMatrixBuffer.get());
		fUpdateModelInstanceMatrixBuffer(animated_model_instances, animatedModelInstanceMatrixBuffer.get());

		std::vector<VkWriteDescriptorSet> writes;

		if (terrains.get_size() > 0)
		{
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(TerrainShaderStruct) * terrains.get_size());

			terrains.iterate([&](int index, void *p, bool &remove) {
				auto t = (TerrainComponent*)p;
				auto n = t->get_parent();
				if (n->is_transform_dirty() || t->is_attribute_dirty())
				{
					auto srcOffset = sizeof(TerrainShaderStruct) * ranges.size();
					TerrainShaderStruct stru;
					stru.coord = n->get_world_coord();
					stru.block_cx = t->get_block_cx();
					stru.block_cy = t->get_block_cy();
					stru.block_size = t->get_block_size();
					stru.terrain_height = t->get_height();
					stru.displacement_height = t->get_displacement_height();
					stru.tessellation_factor = t->get_tessellation_factor();
					stru.tiling_scale = t->get_tiling_scale();
					for (int i = 0; i < 4; i++)
					{
						stru.material_index.v[i] = t->get_material(i) ?
							t->get_material(i)->index : 0;
					}
					stru.material_count = t->get_material_count();
					memcpy(map + srcOffset, &stru, sizeof(TerrainShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(TerrainShaderStruct) * index;
					range.size = sizeof(TerrainShaderStruct);
					ranges.push_back(range);
				}
				if (t->is_blend_image_dirty())
				{
					writes.push_back(ds_terrain->imageWrite(TerrainBlendImageDescriptorBinding,
						index, t->get_blend_image(), colorBorderSampler));
				}
			});

			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copyTo(terrainBuffer.get(), ranges.size(), ranges.data());
		}
		if (waters.get_size() > 0)
		{
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(WaterShaderStruct) * waters.get_size());

			waters.iterate([&](int index, void *p, bool &remove) {
				auto w = (WaterComponent*)p;
				auto n = w->get_parent();
				if (n->is_transform_dirty() || w->is_attribute_dirty())
				{
					auto srcOffset = sizeof(WaterShaderStruct) * ranges.size();
					WaterShaderStruct stru;
					stru.coord = glm::vec3(n->get_world_matrix()[3]);
					stru.block_cx = w->get_block_cx();
					stru.block_cy = w->get_block_cy();
					stru.block_size = w->get_block_size();
					stru.height = w->get_height();
					stru.tessellation_factor = w->get_tessellation_factor();
					stru.tiling_scale = w->get_tiling_scale();
					stru.mapDimension = 1024;
					memcpy(map + srcOffset, &stru, sizeof(WaterShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(WaterShaderStruct) * index;
					range.size = sizeof(WaterShaderStruct);
					ranges.push_back(range);
				}
			});
			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copyTo(waterBuffer.get(), ranges.size(), ranges.data());
		}

		static const auto fUpdateIndirect = [](SpareList &list, Buffer *buffer) {
			if (list.get_size() > 0)
			{
				std::vector<VkDrawIndexedIndirectCommand> commands;
				list.iterate([&](int index, void *p, bool &remove) {
					auto i = (ModelInstanceComponent*)p;
					auto m = i->get_model();
					for (auto &g : m->geometries)
					{
						VkDrawIndexedIndirectCommand command = {};
						command.instanceCount = 1;
						command.indexCount = g->indiceCount;
						command.vertexOffset = m->vertexBase;
						command.firstIndex = m->indiceBase + g->indiceBase;
						command.firstInstance = (index << 8) + g->material->index;
						commands.push_back(command);
					}
				});
				buffer->update(commands.data(), defalut_staging_buffer, sizeof(VkDrawIndexedIndirectCommand) * commands.size());
			}
		};
		if (static_model_instance_count_dirty)
		{
			fUpdateIndirect(static_model_instances, staticObjectIndirectBuffer.get());
			static_model_instance_count_dirty = false;
		}
		if (animated_model_instance_count_dirty)
		{
			fUpdateIndirect(animated_model_instances, animatedObjectIndirectBuffer.get());
			animated_model_instance_count_dirty = false;
		}


		if (light_count_dirty)
		{
			unsigned int count = lights.get_size();
			lightBuffer->update(&count, defalut_staging_buffer, sizeof(int));

			light_count_dirty = false;
		}

		if (lights.get_size() > 0)
		{ // light attribute
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(LightShaderStruct) * lights.get_size());

			lights.iterate([&](int index, void *p, bool &remove) {
				auto l = (LightComponent*)p;
				auto n = l->get_parent();
				LightShaderStruct stru;
				if (l->get_type() == LightTypeParallax)
					stru.coord = glm::vec4(n->get_world_axis()[2], 0.f);
				else
					stru.coord = glm::vec4(n->get_world_coord(), l->get_type());
				stru.color = glm::vec4(l->get_color(), l->get_shadow_index() * 6);
				stru.spotData = glm::vec4(-n->get_world_axis()[2], l->get_range());
				auto srcOffset = sizeof(LightShaderStruct) * ranges.size();
				memcpy(map + srcOffset, &stru, sizeof(LightShaderStruct));
				VkBufferCopy range = {};
				range.srcOffset = srcOffset;
				range.dstOffset = 16 + sizeof(LightShaderStruct) * l->get_light_index();
				range.size = sizeof(LightShaderStruct);
				ranges.push_back(range);
			});
			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copyTo(lightBuffer.get(), ranges.size(), ranges.data());
		}
		if (enable_shadow)
		{
			if (shadow_lights.get_size() > 0)
			{
				shadow_lights.iterate([&](int index, void *p, bool &remove) {
					std::vector<VkBufferCopy> ranges;
					auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(glm::mat4) * shadow_lights.get_size());

					auto l = (LightComponent*)p;
					auto n = l->get_parent();
					if (l->get_type() == LightTypeParallax)
					{
						if (n->is_transform_dirty() || camera->get_parent()->is_transform_dirty())
						{
							glm::vec3 p[8];
							auto cameraCoord = camera->get_parent()->get_world_coord();
							for (int i = 0; i < 8; i++)
								p[i] = camera->get_frustum_points()[i] - cameraCoord;
							auto lighAxis = n->get_world_axis();
							auto axisT = glm::transpose(lighAxis);
							auto vMax = axisT * p[0], vMin = vMax;
							for (int i = 1; i < 8; i++)
							{
								auto tp = axisT * p[i];
								vMax = glm::max(tp, vMax);
								vMin = glm::min(tp, vMin);
							}
							auto halfWidth = (vMax.z - vMin.z) * 0.5f;
							auto halfHeight = (vMax.y - vMin.y) * 0.5f;
							auto halfDepth = glm::max(vMax.x - vMin.x, near_plane) * 0.5f;
							auto center = lighAxis * ((vMax + vMin) * 0.5f) + cameraCoord;
							//auto shadowMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, TKE_NEAR, halfDepth + halfDepth) * 
							//glm::lookAt(center + halfDepth * lighAxis[2], center, lighAxis[1]);
							auto shadowMatrix = glm::mat4(
								1.f, 0.f, 0.f, 0.f,
								0.f, 1.f, 0.f, 0.f,
								0.f, 0.f, 0.5f, 0.f,
								0.f, 0.f, 0.5f, 1.f
							) * glm::ortho(-1.f, 1.f, -1.f, 1.f, near_plane, far_plane) *
								glm::lookAt(camera->get_target() + glm::vec3(0, 0, 100), camera->get_target(), glm::vec3(0, 1, 0));

							auto srcOffset = sizeof(glm::mat4) * ranges.size();
							memcpy(map + srcOffset, &shadowMatrix, sizeof(glm::mat4));
							VkBufferCopy range = {};
							range.srcOffset = srcOffset;
							range.dstOffset = sizeof(glm::mat4) * (index * 6);
							range.size = sizeof(glm::mat4);
							ranges.push_back(range);
						}
					}
					else if (l->get_type() == LightTypePoint)
					{
						if (n->is_transform_dirty())
						{
							glm::mat4 shadowMatrix[6];

							auto coord = n->get_world_coord();
							auto proj = glm::perspective(90.f, 1.f, near_plane, far_plane);
							shadowMatrix[0] = proj * glm::lookAt(coord, coord + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[1] = proj * glm::lookAt(coord, coord + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[2] = proj * glm::lookAt(coord, coord + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[3] = proj * glm::lookAt(coord, coord + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[4] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
							shadowMatrix[5] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
						}
					}
					defalut_staging_buffer->unmap();
					defalut_staging_buffer->copyTo(shadowBuffer.get(), ranges.size(), ranges.data());
				});
			}
		}

		updateDescriptorSets(writes.size(), writes.data());

		if (enable_shadow)
		{
			cb_shad->reset();
			cb_shad->begin();

			shadow_lights.iterate([&](int index, void *p, bool &remove) {
				auto l = (LightComponent*)p;

				static VkClearValue clearValues[] = {
					{ 1.f, 0 },
					{ 1.f, 1.f, 1.f, 1.f }
				};
				cb_shad->beginRenderPass(renderPass_depthC_image32fC, fb_esm[index].get(), clearValues);

				cb_shad->bindVertexBuffer2(vertexStatBuffer.get(), vertexAnimBuffer.get());
				cb_shad->bindIndexBuffer(indexBuffer.get());

				static const auto fDrawDepth = [&](SpareList &list) {
					list.iterate([&](int index, void *p, bool &remove) {
						auto i = (ModelInstanceComponent*)p;
						auto m = i->get_model();
						for (int gId = 0; gId < m->geometries.size(); gId++)
							cb_shad->drawModel(m, gId, 1, ((l->get_shadow_index() * 6) << 28) + (index << 8) + gId);
					});
				};
				if (static_model_instances.get_size() > 0)
				{
					cb_shad->bindPipeline(esm_pipeline);
					VkDescriptorSet sets[] = {
						ds_esm->v,
						ds_material->v
					};
					cb_shad->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
					fDrawDepth(static_model_instances);
				}

				if (animated_model_instances.get_size() > 0)
				{
					cb_shad->bindPipeline(esm_anim_pipeline);
					VkDescriptorSet sets[] = {
						ds_esmAnim->v,
						ds_material->v,
						ds_mrtAnim_bone->v
					};
					cb_shad->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
					fDrawDepth(animated_model_instances);
				}
				cb_shad->endRenderPass();
			});

			cb_shad->end();
		}

		cb_defe->reset();
		cb_defe->begin();

		cb_defe->beginRenderPass(defe_renderpass, this->framebuffer.get());

		cb_defe->bindVertexBuffer2(vertexStatBuffer.get(), vertexAnimBuffer.get());
		cb_defe->bindIndexBuffer(indexBuffer.get());

		// mrt
		// static
		if (static_model_instances.get_size() > 0)
		{
			cb_defe->bindPipeline(mrt_pipeline);
			VkDescriptorSet sets[] = {
				ds_mrt->v,
				ds_material->v
			};
			cb_defe->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb_defe->drawIndirectIndex(staticObjectIndirectBuffer.get(), static_model_instances.get_size());
		}
		// animated
		if (animated_model_instances.get_size())
		{
			cb_defe->bindPipeline(mrt_anim_pipeline);
			VkDescriptorSet sets[] = {
				ds_mrtAnim->v,
				ds_material->v,
				ds_mrtAnim_bone->v
			};
			cb_defe->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb_defe->drawIndirectIndex(animatedObjectIndirectBuffer.get(), animated_model_instances.get_size());
		}
		// terrain
		if (terrains.get_size() > 0)
		{
			cb_defe->bindPipeline(terrain_pipeline);
			VkDescriptorSet sets[] = {
				ds_terrain->v,
				ds_material->v
			};
			cb_defe->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			terrains.iterate([&](int index, void *p, bool &remove) {
				auto t = (TerrainComponent*)p;
				cb_defe->draw(4, 0, (index << 16) + t->get_block_cx() * t->get_block_cx());
			});
		}
		// water
		if (waters.get_size() > 0)
		{
			cb_defe->bindPipeline(water_pipeline);
			cb_defe->bindDescriptorSet(&ds_water->v);
			waters.iterate([&](int index, void *p, bool &remove) {
				auto w = (WaterComponent*)p;
				cb_defe->draw(4, 0, (index << 16) + w->get_block_cx() * w->get_block_cx());
			});
		}

		//cb->imageBarrier(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
		//	VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		//	esmImage.get(), 0, 1, 0, TKE_MAX_SHADOW_COUNT * 8);

		// deferred
		cb_defe->nextSubpass();
		cb_defe->bindPipeline(deferred_pipeline);
		cb_defe->bindDescriptorSet(&ds_defe->v);
		cb_defe->draw(3);

		// compose
		cb_defe->nextSubpass();
		cb_defe->bindPipeline(compose_pipeline);
		cb_defe->bindDescriptorSet(&ds_comp->v);
		cb_defe->draw(3);

		cb_defe->endRenderPass();

		cb_defe->end();
	}

	void DeferredRenderer::add_to_drawlist()
	{
		if (enable_shadow)
			tke::add_to_drawlist(cb_shad->v);
		tke::add_to_drawlist(cb_defe->v);
	}
}
