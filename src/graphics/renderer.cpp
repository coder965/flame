#include "../global.h"
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
#include "../entity/light.h"
#include "../entity/object.h"
#include "../entity/terrain.h"
#include "../entity/water.h"
#include "../application.h"

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
				.vertex_input(&vertexStatInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(engine_path + "shader/plain3d/plain3d.vert", {})
				.addShader(engine_path + "shader/plain3d/plain3d.frag", {}),
				renderPass_depthC_image8, 0);
			pipeline_plain_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexAnimInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(engine_path + "shader/plain3d/plain3d.vert", {"ANIM"})
				.addShader(engine_path + "shader/plain3d/plain3d.frag", {"ANIM"}),
				renderPass_depthC_image8, 0, true);
			pipeline_frontlight = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(engine_path + "shader/plain3d/plain3d.vert", {"USE_NORMAL"})
				.addShader(engine_path + "shader/plain3d/plain3d.frag", {"USE_NORMAL"}),
				renderPass_depthC_image8, 0);
			pipeline_material = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(engine_path + "shader/plain3d/plain3d.vert", {"USE_MATERIAL"})
				.addShader(engine_path + "shader/plain3d/plain3d.frag", {"USE_MATERIAL"}),
				renderPass_depthC_image8, 0);
			pipeline_material_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexAnimInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(engine_path + "shader/plain3d/plain3d.vert", {"ANIM", "USE_MATERIAL"})
				.addShader(engine_path + "shader/plain3d/plain3d.frag", {"ANIM", "USE_MATERIAL"}),
				renderPass_depthC_image8, 0, true);
			pipeline_wireframe = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.polygonMode(VK_POLYGON_MODE_LINE)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/plain3d/plain3d.vert", {})
				.addShader(engine_path + "shader/plain3d/plain3d.frag", {}),
				renderPass_image8, 0);
			pipeline_wireframe_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexAnimInputState)
				.polygonMode(VK_POLYGON_MODE_LINE)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/plain3d/plain3d.vert", {"ANIM"})
				.addShader(engine_path + "shader/plain3d/plain3d.frag", {"ANIM"}),
				renderPass_image8, 0, true);

			first = false;
		}

		cb = std::make_unique<CommandBuffer>();
	}

	void PlainRenderer::render(Framebuffer *framebuffer, bool clear, Camera *camera, DrawData *data)
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
		addCb(cb->v);
	}

	void PlainRenderer::do_render(CommandBuffer *cb, Camera *camera, DrawData *data)
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
		pc.proj = matPerspective;

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

			pc.modelview = camera->getMatInv() * d.mat;
			pc.color = d.color;
			cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
			for (auto &d : d.geo_data)
				cb->drawIndex(d.index_count, d.first_index, d.vertex_offset, d.instance_count, d.first_instance);
		}
	}

	static Pipeline *pipeline_lines;
	bool LinesRenderer::first = true;
	LinesRenderer::LinesRenderer()
	{
		if (first)
		{
			static VkVertexInputBindingDescription bindings = {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};

			static VkVertexInputAttributeDescription attributes[] = {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
			{1, 0, VK_FORMAT_R32G32B32_SFLOAT,	   offsetof(Vertex, color)}
			};

			auto vis = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);

			pipeline_lines = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vis)
				.primitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
				.polygonMode(VK_POLYGON_MODE_LINE)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/plain3d/plain3d_line.vert", {})
				.addShader(engine_path + "shader/plain3d/plain3d_line.frag", {}),
				renderPass_image8, 0);

			first = false;
		}

		cb = std::make_unique<CommandBuffer>();
	}

	void LinesRenderer::render(Framebuffer *framebuffer, bool clear, Camera *camera, void *user_data)
	{
		cb->reset();
		cb->begin();

		auto data = (DrawData*)user_data;

		cb->beginRenderPass(renderPass_image8, framebuffer);

		cb->bindVertexBuffer(data->vertex_buffer);
		cb->bindPipeline(pipeline_lines);

		glm::mat4 mvp = matPerspective * camera->getMatInv();
		cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);
		cb->draw(data->vertex_count);

		cb->endRenderPass();

		cb->end();

		addCb(cb->v);
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
		int blockCx;
		float blockSize;
		float height;
		float tessellationFactor;
		float textureUvFactor;
		float mapDimension;
		unsigned int dummy0;
		unsigned int dummy1;
		unsigned int dummy2;
	};

	static Pipeline *scatteringPipeline;
	static Pipeline *downsamplePipeline;
	static Pipeline *convolvePipeline;
	static Pipeline *copyPipeline;
	static Pipeline *mrtPipeline;
	static Pipeline *mrtAnimPipeline;
	static Pipeline *terrainPipeline;
	static Pipeline *waterPipeline;
	static Pipeline *deferredPipeline;
	static Pipeline *composePipeline;
	static Pipeline *esmPipeline;
	static Pipeline *esmAnimPipeline;
	static RenderPass *defeRenderPass;
	static Image *envrImageDownsample[3] = {};
	bool DeferredRenderer::defe_inited = false;
	bool DeferredRenderer::shad_inited = false;
	DeferredRenderer::DeferredRenderer(bool _enable_shadow, Image *dst)
		:enable_shadow(_enable_shadow), resource(&globalResource)
	{
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
			VkAttachmentReference main_col_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
			VkAttachmentReference mrt_col_ref[] = {
				{2, VK_IMAGE_LAYOUT_GENERAL},
			{3, VK_IMAGE_LAYOUT_GENERAL},
			{4, VK_IMAGE_LAYOUT_GENERAL}
			};
			VkAttachmentReference dep_ref = {1, VK_IMAGE_LAYOUT_GENERAL};
			VkAttachmentReference dst_col_ref = {5, VK_IMAGE_LAYOUT_GENERAL};
			VkSubpassDescription subpasses[] = {
				subpassDesc(ARRAYSIZE(mrt_col_ref), mrt_col_ref, &dep_ref), // mrt
				subpassDesc(1, &main_col_ref),                              // deferred
				subpassDesc(1, &dst_col_ref)                                // compose
			};

			VkSubpassDependency dependencies[] = {
				subpassDependency(0, 1),
				subpassDependency(1, 2)
			};

			defeRenderPass = new RenderPass(ARRAYSIZE(atts), atts, ARRAYSIZE(subpasses), subpasses, ARRAYSIZE(dependencies), dependencies);

			scatteringPipeline = new Pipeline(PipelineCreateInfo()
				.cx(512).cy(256)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/fullscreen.vert", {"USE_UV"})
				.addShader(engine_path + "shader/sky/scattering.frag", {}),
				renderPass_image16, 0);
			downsamplePipeline = new Pipeline(PipelineCreateInfo()
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/fullscreen.vert", {"USE_UV"})
				.addShader(engine_path + "shader/sky/downsample.frag", {})
				, renderPass_image16, 0, true);
			convolvePipeline = new Pipeline(PipelineCreateInfo()
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/fullscreen.vert", {"USE_UV"})
				.addShader(engine_path + "shader/sky/convolve.frag", {}),
				renderPass_image16, 0, true);
			copyPipeline = new Pipeline(PipelineCreateInfo()
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/fullscreen.vert", {"USE_UV"})
				.addShader(engine_path + "shader/copy.frag", {}),
				renderPass_image16, 0, true);
			mrtPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.depth_test(true)
				.depth_write(true)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addShader(engine_path + "shader/deferred/mrt.vert", {})
				.addShader(engine_path + "shader/deferred/mrt.frag", {})
				.addLink("ubo_matrix_", "Matrix.UniformBuffer")
				.addLink("ubo_object_", "ObjectMatrix.UniformBuffer"),
				defeRenderPass, 0);
			mrtAnimPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexAnimInputState)
				.depth_test(true)
				.depth_write(true)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addShader(engine_path + "shader/deferred/mrt.vert", {"ANIM"})
				.addShader(engine_path + "shader/deferred/mrt.frag", {"ANIM"})
				.addLink("ubo_matrix_", "Matrix.UniformBuffer")
				.addLink("ubo_object_", "ObjectMatrix.UniformBuffer"),
				defeRenderPass, 0);
			terrainPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&terrianVertexInputState)
				.patch_control_points(4)
				.depth_test(true)
				.depth_write(true)
				.primitiveTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addShader(engine_path + "shader/deferred/terrain.vert", {})
				.addShader(engine_path + "shader/deferred/terrain.tesc", {})
				.addShader(engine_path + "shader/deferred/terrain.tese", {})
				.addShader(engine_path + "shader/deferred/terrain.frag", {})
				.addLink("ubo_matrix_", "Matrix.UniformBuffer")
				.addLink("ubo_terrain_", "Terrain.UniformBuffer"),
				defeRenderPass, 0);
			waterPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.patch_control_points(4)
				.depth_test(true)
				.depth_write(true)
				.primitiveTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addShader(engine_path + "shader/deferred/water.vert", {})
				.addShader(engine_path + "shader/deferred/water.tesc", {})
				.addShader(engine_path + "shader/deferred/water.tese", {})
				.addShader(engine_path + "shader/deferred/water.frag", {})
				.addLink("ubo_matrix_", "Matrix.UniformBuffer")
				.addLink("ubo_water_", "Water.UniformBuffer"),
				defeRenderPass, 0);
			deferredPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/fullscreen.vert", {"USE_VIEW"})
				.addShader(engine_path + "shader/deferred/deferred.frag", {"USE_PBR", "USE_IBL"})
				.addLink("ubo_constant_", "Constant.UniformBuffer")
				.addLink("ubo_matrix_", "Matrix.UniformBuffer")
				.addLink("img_depth", "Depth.Image", 0, plainUnnormalizedSampler)
				.addLink("img_albedo_alpha", "AlbedoAlpha.Image", 0, plainUnnormalizedSampler)
				.addLink("img_normal_height", "NormalHeight.Image", 0, plainUnnormalizedSampler)
				.addLink("img_spec_roughness", "SpecRoughness.Image", 0, plainUnnormalizedSampler)
				.addLink("ubo_light_", "Light.UniformBuffer")
				.addLink("img_envr", "Envr.Image", 0, colorSampler)
				.addLink("ubo_ambient_", "Ambient.UniformBuffer")
				.addLink("ubo_shadow_", "Shadow.UniformBuffer"),
				defeRenderPass, 1);
			composePipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(engine_path + "shader/fullscreen.vert", {})
				.addShader(engine_path + "shader/compose/compose.frag", {})
				.addLink("img_source", "Main.Image", 0, plainUnnormalizedSampler),
				defeRenderPass, 2);

			defe_inited = true;
		}

		cb = std::make_unique<CommandBuffer>();

		matrixBuffer = std::make_unique<UniformBuffer>(sizeof MatrixBufferShaderStruct);
		objectMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxObjectCount);
		terrainBuffer = std::make_unique<UniformBuffer>(sizeof(TerrainShaderStruct) * MaxTerrainCount);
		waterBuffer = std::make_unique<UniformBuffer>(sizeof(WaterShaderStruct) * MaxWaterCount);
		lightBuffer = std::make_unique<UniformBuffer>(sizeof(LightBufferShaderStruct));
		ambientBuffer = std::make_unique<UniformBuffer>(sizeof AmbientBufferShaderStruct);
		staticObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * MaxIndirectCount);
		animatedObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * MaxIndirectCount);

		envrImage = std::make_unique<Image>(EnvrSizeCx, EnvrSizeCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 4);
		for (int i = 0; i < 3; i++)
			envrImageDownsample[i] = new Image(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1),
				VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mainImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		depthImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		albedoAlphaImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		normalHeightImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		specRoughnessImage = std::make_unique<Image>(res_cx, res_cy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		ds_mrt = std::make_unique<DescriptorSet>(mrtPipeline);
		ds_mrtAnim = std::make_unique<DescriptorSet>(mrtAnimPipeline);
		ds_mrtAnim_bone = std::make_unique<DescriptorSet>(mrtAnimPipeline, 2);
		ds_terrain = std::make_unique<DescriptorSet>(terrainPipeline);
		ds_water = std::make_unique<DescriptorSet>(waterPipeline);
		ds_defe = std::make_unique<DescriptorSet>(deferredPipeline);
		ds_comp = std::make_unique<DescriptorSet>(composePipeline);

		resource.setImage(envrImage.get(), "Envr.Image");
		resource.setImage(mainImage.get(), "Main.Image");
		resource.setImage(depthImage.get(), "Depth.Image");
		resource.setImage(albedoAlphaImage.get(), "AlbedoAlpha.Image");
		resource.setImage(normalHeightImage.get(), "NormalHeight.Image");
		resource.setImage(specRoughnessImage.get(), "SpecRoughness.Image");

		resource.setBuffer(matrixBuffer.get(), "Matrix.UniformBuffer");
		resource.setBuffer(objectMatrixBuffer.get(), "ObjectMatrix.UniformBuffer");
		resource.setBuffer(terrainBuffer.get(), "Terrain.UniformBuffer");
		resource.setBuffer(waterBuffer.get(), "Water.UniformBuffer");
		resource.setBuffer(lightBuffer.get(), "Light.UniformBuffer");
		resource.setBuffer(ambientBuffer.get(), "Ambient.UniformBuffer");
		resource.setBuffer(staticObjectIndirectBuffer.get(), "Scene.Static.IndirectBuffer");
		resource.setBuffer(animatedObjectIndirectBuffer.get(), "Scene.Animated.IndirectBuffer");

		mrtPipeline->linkDescriptors(ds_mrt.get(), &resource);
		mrtAnimPipeline->linkDescriptors(ds_mrtAnim.get(), &resource);
		terrainPipeline->linkDescriptors(ds_terrain.get(), &resource);
		waterPipeline->linkDescriptors(ds_water.get(), &resource);
		deferredPipeline->linkDescriptors(ds_defe.get(), &resource);
		composePipeline->linkDescriptors(ds_comp.get(), &resource);

		if (enable_shadow)
		{
			if (!shad_inited)
			{
				esmPipeline = new Pipeline(PipelineCreateInfo()
					.cx(2048).cy(2048)
					.vertex_input(&vertexStatInputState)
					.depth_test(true)
					.depth_write(true)
					.addShader(engine_path + "shader/esm/esm.vert", {})
					.addShader(engine_path + "shader/esm/esm.frag", {})
					.addLink("ubo_constant_", "Constant.UniformBuffer")
					.addLink("ubo_object_", "ObjectMatrix.UniformBuffer")
					.addLink("u_shadow_", "Shadow.UniformBuffer"),
					renderPass_depthC_image8C, 0);
				esmAnimPipeline = new Pipeline(PipelineCreateInfo()
					.cx(2048).cy(2048)
					.vertex_input(&vertexAnimInputState)
					.depth_test(true)
					.depth_write(true)
					.addShader(engine_path + "shader/esm/esm.vert", {"ANIM"})
					.addShader(engine_path + "shader/esm/esm.frag", {"ANIM"})
					.addLink("ubo_constant_", "Constant.UniformBuffer")
					.addLink("ubo_object_", "ObjectMatrix.UniformBuffer")
					.addLink("u_shadow_", "Shadow.UniformBuffer"),
					renderPass_depthC_image8C, 0);

				shad_inited = true;
			}

			shadowBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxShadowCount);

			esmImage = std::make_unique<Image>(ShadowMapCx, ShadowMapCy, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, MaxShadowCount * 6);
			esmDepthImage = std::make_unique<Image>(ShadowMapCx, ShadowMapCy, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			for (int i = 0; i < MaxShadowCount * 6; i++)
			{
				VkImageView views[] = {
					esmImage->getView(0, 1, i),
					esmDepthImage->getView()
				};
				fb_esm[i] = getFramebuffer(ShadowMapCx, ShadowMapCy, renderPass_depthC_image32fC, TK_ARRAYSIZE(views), views);
			}

			ds_esm = std::make_unique<DescriptorSet>(esmPipeline);
			ds_esmAnim = std::make_unique<DescriptorSet>(esmAnimPipeline);

			resource.setBuffer(shadowBuffer.get(), "Shadow.UniformBuffer");

			esmPipeline->linkDescriptors(ds_esm.get(), &resource);
			esmAnimPipeline->linkDescriptors(ds_esmAnim.get(), &resource);
		}


		{
			VkImageView views[] = {
				mainImage->getView(),
				depthImage->getView(),
				albedoAlphaImage->getView(),
				normalHeightImage->getView(),
				specRoughnessImage->getView(),
				dst->getView(),
			};
			framebuffer = getFramebuffer(res_cx, res_cy, defeRenderPass, ARRAYSIZE(views), views);
		}
	}

	void DeferredRenderer::render(Framebuffer *framebuffer, bool clear, Camera *camera, void *user_data)
	{
		auto scene = (Scene*)user_data;

		{ // always update the matrix buffer
			MatrixBufferShaderStruct stru;
			stru.proj = matPerspective;
			stru.projInv = matPerspectiveInv;
			stru.view = scene->camera.getMatInv();
			stru.viewInv = scene->camera.getMat();
			stru.projView = stru.proj * stru.view;
			stru.projViewRotate = stru.proj * glm::mat4(glm::mat3(stru.view));
			memcpy(stru.frustumPlanes, scene->camera.frustumPlanes, sizeof(MatrixBufferShaderStruct::frustumPlanes));
			stru.viewportDim = glm::vec2(res_cx, res_cy);
			matrixBuffer->update(&stru, defalut_staging_buffer);
		}

		if (scene->needUpdateSky)
		{
			auto funUpdateIBL = [&]() {
				for (int i = 0; i < envrImage->levels.size() - 1; i++)
				{
					auto cb = begineOnceCommandBuffer();
					auto fb = getFramebuffer(envrImageDownsample[i], renderPass_image16);

					cb->beginRenderPass(renderPass_image16, fb.get());
					cb->bindPipeline(downsamplePipeline);
					cb->setViewportAndScissor(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1));
					auto size = glm::vec2(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1));
					cb->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof glm::vec2, &size);
					updateDescriptorSets(1, &downsamplePipeline->descriptorSet->imageWrite(0, 0, i == 0 ? envrImage.get() : envrImageDownsample[i - 1], plainSampler));
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
					cb->bindPipeline(convolvePipeline);
					auto data = 1.f + 1024.f - 1024.f * (i / 3.f);
					cb->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &data);
					cb->setViewportAndScissor(EnvrSizeCx >> i, EnvrSizeCy >> i);
					updateDescriptorSets(1, &convolvePipeline->descriptorSet->imageWrite(0, 0, envrImageDownsample[i - 1], plainSampler));
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
				case SkyType::atmosphere_scattering:
				{
					auto as = (SkyAtmosphereScattering*)scene->sky.get();

					auto cb = begineOnceCommandBuffer();
					auto fb = getFramebuffer(envrImage.get(), renderPass_image16);

					cb->beginRenderPass(renderPass_image16, fb.get());
					cb->bindPipeline(scatteringPipeline);
					auto euler = as->sun_light->getEuler();
					auto dir = glm::vec2(euler.x, euler.z);
					cb->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(dir), &dir);
					cb->draw(3);
					cb->endRenderPass();

					endOnceCommandBuffer(cb);

					funUpdateIBL();

					break;
				}
				case SkyType::panorama:
				{
					auto pa = (SkyPanorama*)scene->sky.get();

					if (pa->panoImage)
					{
						auto cb = begineOnceCommandBuffer();
						auto fb = getFramebuffer(envrImage.get(), renderPass_image16);

						cb->beginRenderPass(renderPass_image16, fb.get());
						cb->bindPipeline(copyPipeline);
						cb->setViewportAndScissor(EnvrSizeCx, EnvrSizeCy);
						updateDescriptorSets(1, &copyPipeline->descriptorSet->imageWrite(0, 0, pa->panoImage.get(), colorSampler));
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
		}
		if (scene->needUpdateAmbientBuffer)
		{
			AmbientBufferShaderStruct stru;
			stru.color = scene->ambientColor;
			stru.envr_max_mipmap = envrImage->levels.size() - 1;
			stru.fogcolor = glm::vec4(scene->fogColor, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
			ambientBuffer->update(&stru, defalut_staging_buffer);
		}
		if (scene->objects.size() > 0)
		{
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(glm::mat4) * scene->objects.size());
			int index = 0;

			for (auto &o : scene->objects)
			{
				if (o->changed)
				{
					auto srcOffset = sizeof(glm::mat4) * ranges.size();
					memcpy(map + srcOffset, &o->getMat(), sizeof(glm::mat4));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(glm::mat4) * index;
					range.size = sizeof(glm::mat4);
					ranges.push_back(range);
				}
				o->sceneIndex = index;
				index++;
			}
			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copyTo(objectMatrixBuffer.get(), ranges.size(), ranges.data());
		}

		std::vector<VkWriteDescriptorSet> writes;

		if (scene->terrains.size() > 0)
		{
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(TerrainShaderStruct) * scene->terrains.size());

			auto index = 0;
			for (auto &t : scene->terrains)
			{
				t->sceneIndex = index;
				if (t->changed)
				{
					auto srcOffset = sizeof(TerrainShaderStruct) * ranges.size();
					TerrainShaderStruct stru;
					stru.coord = t->getCoord();
					stru.block_cx = t->block_cx;
					stru.block_cy = t->block_cy;
					stru.block_size = t->block_size;
					stru.terrain_height = t->height;
					stru.displacement_height = t->displacement_height;
					stru.tessellation_factor = t->tessellation_factor;
					stru.tiling_scale = t->tiling_scale;
					for (int i = 0; i < 4; i++)
					{
						stru.material_index.v[i] = t->materials[i] ?
							t->materials[0]->index : 0;
					}
					stru.material_count = t->material_count;
					memcpy(map + srcOffset, &stru, sizeof(TerrainShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(TerrainShaderStruct) * index;
					range.size = sizeof(TerrainShaderStruct);
					ranges.push_back(range);

					writes.push_back(ds_terrain->imageWrite(TerrainBlendImageDescriptorBinding, 
						index, t->blendMap.get(),  colorBorderSampler));
				}
				index++;
			}

			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copyTo(terrainBuffer.get(), ranges.size(), ranges.data());
		}
		if (scene->waters.size() > 0)
		{
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(WaterShaderStruct) * scene->waters.size());

			auto index = 0;
			for (auto &w : scene->waters)
			{
				w->sceneIndex = index;
				if (w->changed)
				{
					auto srcOffset = sizeof(WaterShaderStruct) * ranges.size();
					WaterShaderStruct stru;
					stru.coord = w->getCoord();
					stru.blockCx = w->blockCx;
					stru.blockSize = w->blockSize;
					stru.height = w->height;
					stru.tessellationFactor = w->tessellationFactor;
					stru.textureUvFactor = w->textureUvFactor;
					stru.mapDimension = 1024;
					memcpy(map + srcOffset, &stru, sizeof(WaterShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(WaterShaderStruct) * index;
					range.size = sizeof(WaterShaderStruct);
					ranges.push_back(range);
				}
				index++;
			}
			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copyTo(waterBuffer.get(), ranges.size(), ranges.data());
		}

		std::vector<Object*> staticObjects;
		std::vector<Object*> animatedObjects;
		if (scene->object_count_dirty)
		{
			staticObjects.clear();
			animatedObjects.clear();

			if (scene->objects.size() > 0)
			{
				std::vector<VkDrawIndexedIndirectCommand> staticCommands;
				std::vector<VkDrawIndexedIndirectCommand> animatedCommands;

				int staticIndex = 0;
				int animatedIndex = 0;

				for (auto &o : scene->objects)
				{
					auto m = o->model;

					if (!m->vertex_skeleton)
					{
						for (auto &g : m->geometries)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = g->indiceCount;
							command.vertexOffset = m->vertexBase;
							command.firstIndex = m->indiceBase + g->indiceBase;
							command.firstInstance = (staticIndex << 8) + g->material->index;

							staticCommands.push_back(command);
						}

						staticObjects.push_back(o.get());
						staticIndex++;
					}
					else
					{
						for (auto &g : m->geometries)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = g->indiceCount;
							command.vertexOffset = m->vertexBase;
							command.firstIndex = m->indiceBase + g->indiceBase;
							command.firstInstance = (animatedIndex << 8) + g->material->index;

							animatedCommands.push_back(command);
						}

						writes.push_back(ds_mrtAnim_bone->bufferWrite(0, animatedIndex, o->animationComponent->bone_buffer.get()));

						animatedObjects.push_back(o.get());
						animatedIndex++;
					}
				}

				staticIndirectCount = staticCommands.size();
				animatedIndirectCount = animatedCommands.size();

				if (staticCommands.size() > 0)
					staticObjectIndirectBuffer->update(staticCommands.data(), defalut_staging_buffer, sizeof(VkDrawIndexedIndirectCommand) * staticCommands.size());
				if (animatedCommands.size() > 0)
					animatedObjectIndirectBuffer->update(animatedCommands.data(), defalut_staging_buffer, sizeof(VkDrawIndexedIndirectCommand) * animatedCommands.size());
			}
		}

		std::vector<Light*> shadowLights;
		if (enable_shadow)
		{
			if (scene->lights.size() > 0)
			{
				auto shadowIndex = 0;
				std::vector<VkBufferCopy> ranges;
				auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(glm::mat4) * scene->lights.size());

				for (auto &l : scene->lights)
				{
					if (!l->shadow)
					{
						l->sceneShadowIndex = -1;
						continue;
					}

					l->sceneShadowIndex = shadowIndex;
					shadowLights.push_back(l.get());

					if (l->type == LightType::parallax)
					{
						if (l->changed || scene->camera.changed)
						{
							glm::vec3 p[8];
							auto cameraCoord = scene->camera.coord;
							for (int i = 0; i < 8; i++) p[i] = scene->camera.frustumPoints[i] - cameraCoord;
							auto lighAxis = l->getAxis();
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
							glm::lookAt(center + halfDepth * lighAxis[2], center, lighAxis[1]);
							auto shadowMatrix = glm::mat4(1.f, 0.f, 0.f, 0.f,
								0.f, 1.f, 0.f, 0.f,
								0.f, 0.f, 0.5f, 0.f,
								0.f, 0.f, 0.5f, 1.f) *
								glm::ortho(-1.f, 1.f, -1.f, 1.f, near_plane, far_plane) *
								glm::lookAt(scene->camera.target + glm::vec3(0, 0, 100), scene->camera.target, glm::vec3(0, 1, 0));

							auto srcOffset = sizeof(glm::mat4) * ranges.size();
							memcpy(map + srcOffset, &shadowMatrix, sizeof(glm::mat4));
							VkBufferCopy range = {};
							range.srcOffset = srcOffset;
							range.dstOffset = sizeof(glm::mat4) * shadowIndex;
							range.size = sizeof(glm::mat4);
							ranges.push_back(range);

							writes.push_back(ds_defe->imageWrite(ShadowImageDescriptorBinding, shadowIndex, esmImage.get(), colorSampler, 0, 1, shadowIndex, 1));
						}
						shadowIndex += 6;
					}
					else if (l->type == LightType::point)
					{
						if (l->changed)
						{
							glm::mat4 shadowMatrix[6];

							auto coord = l->getCoord();
							auto proj = glm::perspective(90.f, 1.f, near_plane, far_plane);
							shadowMatrix[0] = proj * glm::lookAt(coord, coord + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[1] = proj * glm::lookAt(coord, coord + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[2] = proj * glm::lookAt(coord, coord + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[3] = proj * glm::lookAt(coord, coord + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[4] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
							shadowMatrix[5] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
						}
						shadowIndex += 6;
					}
				}
				defalut_staging_buffer->unmap();
				defalut_staging_buffer->copyTo(shadowBuffer.get(), ranges.size(), ranges.data());
			}
		}

		if (scene->light_count_dirty)
		{
			unsigned int count = scene->lights.size();
			lightBuffer->update(&count, defalut_staging_buffer, sizeof(int));
		}

		if (scene->lights.size() > 0)
		{ // light attribute
			int lightIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)defalut_staging_buffer->map(0, sizeof(LightShaderStruct) * scene->lights.size());
			for (auto &l : scene->lights)
			{
				l->sceneIndex = lightIndex;
				if (l->changed)
				{
					LightShaderStruct stru;
					if (l->type == LightType::parallax)
						stru.coord = glm::vec4(l->getAxis()[2], 0.f);
					else
						stru.coord = glm::vec4(l->getCoord(), l->type);
					stru.color = glm::vec4(l->color, l->sceneShadowIndex);
					stru.spotData = glm::vec4(-l->getAxis()[2], l->range);
					auto srcOffset = sizeof(LightShaderStruct) * ranges.size();
					memcpy(map + srcOffset, &stru, sizeof(LightShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = 16 + sizeof(LightShaderStruct) * lightIndex;
					range.size = sizeof(LightShaderStruct);
					ranges.push_back(range);
				}
				lightIndex++;
			}
			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copyTo(lightBuffer.get(), ranges.size(), ranges.data());
		}

		updateDescriptorSets(writes.size(), writes.data());

		cb->reset();
		cb->begin();

		if (enable_shadow)
		{
			for (int i = 0; i < shadowLights.size(); i++)
			{
				auto l = shadowLights[i];

				VkClearValue clearValues[] = {
					{1.f, 0},
				{1.f, 1.f, 1.f, 1.f}
				};
				cb->beginRenderPass(renderPass_depthC_image32fC, fb_esm[i].get(), clearValues);

				cb->bindVertexBuffer2(vertexStatBuffer.get(), vertexAnimBuffer.get());
				cb->bindIndexBuffer(indexBuffer.get());

				// static
				if (staticObjects.size() > 0)
				{
					cb->bindPipeline(esmPipeline);
					VkDescriptorSet sets[] = {
						ds_esm->v,
						ds_material->v
					};
					cb->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
					for (int oId = 0; oId < staticObjects.size(); oId++)
					{
						auto o = staticObjects[oId];
						auto m = o->model;
						for (int gId = 0; gId < m->geometries.size(); gId++)
							cb->drawModel(m.get(), gId, 1, (i << 28) + (oId << 8) + gId);
					}
				}
				// animated
				if (animatedObjects.size() > 0)
				{
					cb->bindPipeline(esmAnimPipeline);
					VkDescriptorSet sets[] = {
						ds_esmAnim->v,
						ds_material->v,
						ds_mrtAnim_bone->v
					};
					cb->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
					for (int oId = 0; oId < animatedObjects.size(); oId++)
					{
						auto o = animatedObjects[oId];
						auto m = o->model;
						for (int gId = 0; gId < m->geometries.size(); gId++)
							cb->drawModel(m.get(), gId, 1, (i << 28) + (oId << 8) + gId);
					}
				}
				cb->endRenderPass();
			}
		}

		cb->beginRenderPass(defeRenderPass, this->framebuffer.get());

		cb->bindVertexBuffer2(vertexStatBuffer.get(), vertexAnimBuffer.get());
		cb->bindIndexBuffer(indexBuffer.get());

		// mrt
		// static
		if (staticIndirectCount > 0)
		{
			cb->bindPipeline(mrtPipeline);
			VkDescriptorSet sets[] = {
				ds_mrt->v,
				ds_material->v
			};
			cb->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb->drawIndirectIndex(staticObjectIndirectBuffer.get(), staticIndirectCount);
		}
		// animated
		if (animatedIndirectCount)
		{
			cb->bindPipeline(mrtAnimPipeline);
			VkDescriptorSet sets[] = {
				ds_mrtAnim->v,
				ds_material->v,
				ds_mrtAnim_bone->v
			};
			cb->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb->drawIndirectIndex(animatedObjectIndirectBuffer.get(), animatedIndirectCount);
		}
		// terrain
		if (scene->terrains.size() > 0)
		{
			cb->bindPipeline(terrainPipeline);
			VkDescriptorSet sets[] = {
				ds_terrain->v,
				ds_material->v
			};
			cb->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			int index = 0;
			for (auto &t : scene->terrains)
			{
				cb->draw(4, 0, (index << 16) + t->block_cx * t->block_cx);
				index++;
			}
		}
		// water
		if (scene->waters.size() > 0)
		{
			cb->bindPipeline(waterPipeline);
			cb->bindDescriptorSet(&ds_water->v);
			int index = 0;
			for (auto &w : scene->waters)
			{
				cb->draw(4, 0, (index << 16) + w->blockCx * w->blockCx);
				index++;
			}
		}

		//cb->imageBarrier(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
		//	VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		//	esmImage.get(), 0, 1, 0, TKE_MAX_SHADOW_COUNT * 8);

		// deferred
		cb->nextSubpass();
		cb->bindPipeline(deferredPipeline);
		cb->bindDescriptorSet(&ds_defe->v);
		cb->draw(3);

		// compose
		cb->nextSubpass();
		cb->bindPipeline(composePipeline);
		cb->bindDescriptorSet(&ds_comp->v);
		cb->draw(3);

		cb->endRenderPass();

		cb->end();

		addCb(cb->v);
	}
}
