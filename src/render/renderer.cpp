#include "renderer.h"
#include "synchronization.h"
#include "renderpass.h"

namespace tke
{
	Renderer::Renderer()
	{
		cb = std::make_unique<CommandBuffer>();
		renderFinished = createEvent();
	}

	Renderer::~Renderer()
	{
		destroyEvent(renderFinished);
	}

	void Renderer::render(FrameCommandBufferList *cb_list, Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data)
	{
		cb->reset();
		cb->begin();
		//if (cb_list && cb_list->last_event)
		//	cb->waitEvents(1, &cb_list->last_event);

		do_render(framebuffer, clear, camera, count, user_data);

		//if (cb_list && cb_list->last_event)
		//{
		//	cb->resetEvent(cb_list->last_event);
		//	cb->setEvent(renderFinished);
		//}
		cb->end();
		if (cb_list)
			cb_list->add(cb->v, renderFinished);
	}

	static Pipeline *pipeline_plain;
	static Pipeline *pipeline_plain_anim;
	static Pipeline *pipeline_frontlight;
	static Pipeline *pipeline_texture;
	static Pipeline *pipeline_texture_anim;
	static Pipeline *pipeline_wireframe;
	static Pipeline *pipeline_wireframe_anim;
	bool PlainRenderer::first = true;
	UniformBuffer *PlainRenderer::last_bone_buffer;
	PlainRenderer::PlainRenderer()
	{
		if (first)
		{
			pipeline_plain = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(enginePath + "shader/plain3d/plain3d.vert", {})
				.addShader(enginePath + "shader/plain3d/plain3d.frag", {}),
				renderPass_depthC_image8, 0);
			pipeline_plain_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexAnimInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(enginePath + "shader/plain3d/plain3d.vert", {"ANIM"})
				.addShader(enginePath + "shader/plain3d/plain3d.frag", {"ANIM"}),
				renderPass_depthC_image8, 0, true);
			pipeline_frontlight = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(enginePath + "shader/plain3d/plain3d.vert", {"USE_NORMAL"})
				.addShader(enginePath + "shader/plain3d/plain3d.frag", {"USE_NORMAL"}),
				renderPass_depthC_image8, 0);
			pipeline_texture = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(enginePath + "shader/plain3d/plain3d.vert", {"USE_TEX"})
				.addShader(enginePath + "shader/plain3d/plain3d.frag", {"USE_TEX"}),
				renderPass_depthC_image8, 0);
			pipeline_texture_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexAnimInputState)
				.depth_test(true)
				.depth_write(true)
				.addShader(enginePath + "shader/plain3d/plain3d.vert", {"ANIM", "USE_TEX"})
				.addShader(enginePath + "shader/plain3d/plain3d.frag", {"ANIM", "USE_TEX"}),
				renderPass_depthC_image8, 0, true);
			pipeline_wireframe = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.polygonMode(VK_POLYGON_MODE_LINE)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(enginePath + "shader/plain3d/plain3d.vert", {})
				.addShader(enginePath + "shader/plain3d/plain3d.frag", {}),
				renderPass_image8, 0);
			pipeline_wireframe_anim = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexAnimInputState)
				.polygonMode(VK_POLYGON_MODE_LINE)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(enginePath + "shader/plain3d/plain3d.vert", {"ANIM"})
				.addShader(enginePath + "shader/plain3d/plain3d.frag", {"ANIM"}),
				renderPass_image8, 0, true);

			first = false;
		}
	}

	void PlainRenderer::do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int _count, void *user_data)
	{
		auto mode = TK_HIGH(_count);
		auto count = TK_LOW(_count);

		cb->beginRenderPass(clear ?
			(mode == 3 ? renderPass_image8C : renderPass_depthC_image8C)
			: (mode == 3 ? renderPass_image8 : renderPass_depthC_image8)
			, framebuffer);
		render_to(cb.get(), mode, camera, count, (DrawData*)user_data);
		cb->endRenderPass();
	}

	void PlainRenderer::render_to(CommandBuffer *cb, int mode, Camera *camera, int count, DrawData *data)
	{
		cb->bindVertexBuffer2(vertexStatBuffer, vertexAnimBuffer);
		cb->bindIndexBuffer(indexBuffer);

		struct
		{
			glm::mat4 modelview;
			glm::mat4 proj;
			glm::vec4 color;
		}pc;
		pc.proj = matPerspective;

		for (int i = 0; i < count; i++)
		{
			auto &d = data[i];
			auto model = d.model;
			auto animated = model->animated;

			switch (mode)
			{
				case 0:
					if (!animated)
						cb->bindPipeline(pipeline_plain);
					else
					{
						cb->bindPipeline(pipeline_plain_anim);
						if (last_bone_buffer != d.bone_buffer)
						{
							updateDescriptorSets(1, &pipeline_plain_anim->descriptorSet->bufferWrite(0, 0, d.bone_buffer));
							last_bone_buffer = d.bone_buffer;
						}
						cb->bindDescriptorSet();
					}
					break;
				case 1:
					cb->bindPipeline(pipeline_frontlight);
					break;
				case 2:
					cb->bindPipeline(!animated ? pipeline_texture : pipeline_texture_anim);
					break;
				case 3:
					cb->bindPipeline(!animated ? pipeline_wireframe : pipeline_wireframe_anim);
					break;
			}

			pc.modelview = camera->getMatInv() * d.mat;
			pc.color = d.color;
			cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
			cb->drawModel(model);
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
				.addShader(enginePath + "shader/plain3d/plain3d_line.vert", {})
				.addShader(enginePath + "shader/plain3d/plain3d_line.frag", {}),
				renderPass_image8, 0);

			first = false;
		}
	}

	void LinesRenderer::do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data)
	{
		cb->beginRenderPass(renderPass_image8, framebuffer);

		cb->bindVertexBuffer((OnceVertexBuffer*)user_data);
		cb->bindPipeline(pipeline_lines);

		glm::mat4 mvp = matPerspective * camera->getMatInv();
		cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);
		cb->draw(count);

		cb->endRenderPass();
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
				.addShader(enginePath + "shader/fullscreenUv.vert", {})
				.addShader(enginePath + "shader/sky/scattering.frag", {}),
				renderPass_image16, 0);
			downsamplePipeline = new Pipeline(PipelineCreateInfo()
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(enginePath + "shader/fullscreenUv.vert", {})
				.addShader(enginePath + "shader/sky/downsample.frag", {})
				, renderPass_image16, 0, true);
			convolvePipeline = new Pipeline(PipelineCreateInfo()
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(enginePath + "shader/fullscreenUv.vert", {})
				.addShader(enginePath + "shader/sky/convolve.frag", {}),
				renderPass_image16, 0, true);
			mrtPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexStatInputState)
				.depth_test(true)
				.depth_write(true)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addShader(enginePath + "shader/deferred/mrt.vert", {})
				.addShader(enginePath + "shader/deferred/mrt.frag", {})
				.addLink("MATRIX", "Matrix.UniformBuffer")
				.addLink("OBJECT", "StaticObjectMatrix.UniformBuffer")
				.addLink("MATERIAL", "Material.UniformBuffer"),
				defeRenderPass, 0);
			mrtAnimPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.vertex_input(&vertexAnimInputState)
				.depth_test(true)
				.depth_write(true)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addShader(enginePath + "shader/deferred/mrt.vert", {"ANIM"})
				.addShader(enginePath + "shader/deferred/mrt.frag", {"ANIM"})
				.addLink("MATRIX", "Matrix.UniformBuffer")
				.addLink("OBJECT", "AnimatedObjectMatrix.UniformBuffer")
				.addLink("MATERIAL", "Material.UniformBuffer"),
				defeRenderPass, 0);
			terrainPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.patch_control_points(4)
				.depth_test(true)
				.depth_write(true)
				.primitiveTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addBlendAttachmentState(false)
				.addShader(enginePath + "shader/deferred/terrain.vert", {})
				.addShader(enginePath + "shader/deferred/terrain.tesc", {})
				.addShader(enginePath + "shader/deferred/terrain.tese", {})
				.addShader(enginePath + "shader/deferred/terrain.frag", {})
				.addLink("MATRIX", "Matrix.UniformBuffer")
				.addLink("TERRAIN", "Terrain.UniformBuffer"),
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
				.addShader(enginePath + "shader/deferred/water.vert", {})
				.addShader(enginePath + "shader/deferred/water.tesc", {})
				.addShader(enginePath + "shader/deferred/water.tese", {})
				.addShader(enginePath + "shader/deferred/water.frag", {})
				.addLink("MATRIX", "Matrix.UniformBuffer")
				.addLink("WATER", "Water.UniformBuffer"),
				defeRenderPass, 0);
			deferredPipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(enginePath + "shader/fullscreenView.vert", {})
				.addShader(enginePath + "shader/deferred/deferred.frag", {"USE_PBR", "USE_IBL"})
				.addLink("CONSTANT", "Constant.UniformBuffer")
				.addLink("MATRIX", "Matrix.UniformBuffer")
				.addLink("AMBIENT", "Ambient.UniformBuffer")
				.addLink("LIGHT", "Light.UniformBuffer")
				.addLink("envrSampler", "Envr.Image", 0, colorSampler)
				.addLink("depthSampler", "Depth.Image", 0, plainUnnormalizedSampler)
				.addLink("albedoAlphaSampler", "AlbedoAlpha.Image", 0, plainUnnormalizedSampler)
				.addLink("normalHeightSampler", "NormalHeight.Image", 0, plainUnnormalizedSampler)
				.addLink("specRoughnessSampler", "SpecRoughness.Image", 0, plainUnnormalizedSampler)
				.addLink("SHADOW", "Shadow.UniformBuffer"),
				defeRenderPass, 1);
			composePipeline = new Pipeline(PipelineCreateInfo()
				.cx(-1).cy(-1)
				.cullMode(VK_CULL_MODE_NONE)
				.addShader(enginePath + "shader/fullscreen.vert", {})
				.addShader(enginePath + "shader/compose/compose.frag", {})
				.addLink("source", "Main.Image", 0, plainUnnormalizedSampler),
				defeRenderPass, 2);

			defe_inited = true;
		}

		matrixBuffer = std::make_unique<UniformBuffer>(sizeof MatrixBufferShaderStruct);
		staticObjectMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxStaticObjectCount);
		animatedObjectMatrixBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * MaxAnimatedObjectCount);
		terrainBuffer = std::make_unique<UniformBuffer>(sizeof TerrainShaderStruct);
		waterBuffer = std::make_unique<UniformBuffer>(sizeof(WaterShaderStruct) * MaxWaterCount);
		lightBuffer = std::make_unique<UniformBuffer>(sizeof(LightBufferShaderStruct));
		ambientBuffer = std::make_unique<UniformBuffer>(sizeof AmbientBufferShaderStruct);
		staticObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * MaxIndirectCount);
		animatedObjectIndirectBuffer = std::make_unique<IndirectIndexBuffer>(sizeof(VkDrawIndexedIndirectCommand) * MaxIndirectCount);

		envrImage = std::make_unique<Image>(EnvrSizeCx, EnvrSizeCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 4);
		for (int i = 0; i < 3; i++)
			envrImageDownsample[i] = new Image(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1),
				VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mainImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		depthImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		albedoAlphaImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		normalHeightImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		specRoughnessImage = std::make_unique<Image>(resCx, resCy, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

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
		resource.setBuffer(staticObjectMatrixBuffer.get(), "StaticObjectMatrix.UniformBuffer");
		resource.setBuffer(animatedObjectMatrixBuffer.get(), "AnimatedObjectMatrix.UniformBuffer");
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
					.addShader(enginePath + "shader/esm/esm.vert", {})
					.addShader(enginePath + "shader/esm/esm.frag", {})
					.addLink("CONSTANT", "Constant.UniformBuffer")
					.addLink("OBJECT", "StaticObjectMatrix.UniformBuffer")
					.addLink("SHADOW", "Shadow.UniformBuffer")
					.addLink("MATERIAL", "Material.UniformBuffer"),
					renderPass_depthC_image8C, 0);
				esmAnimPipeline = new Pipeline(PipelineCreateInfo()
					.cx(2048).cy(2048)
					.vertex_input(&vertexAnimInputState)
					.depth_test(true)
					.depth_write(true)
					.addShader(enginePath + "shader/esm/esm.vert", {"ANIM"})
					.addShader(enginePath + "shader/esm/esm.frag", {"ANIM"})
					.addLink("CONSTANT", "Constant.UniformBuffer")
					.addLink("OBJECT", "AnimatedObjectMatrix.UniformBuffer")
					.addLink("SHADOW", "Shadow.UniformBuffer")
					.addLink("MATERIAL", "Material.UniformBuffer"),
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
			framebuffer = getFramebuffer(resCx, resCy, defeRenderPass, ARRAYSIZE(views), views);
		}
	}

	void DeferredRenderer::do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data)
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
			stru.viewportDim = glm::vec2(resCx, resCy);
			matrixBuffer->update(&stru, stagingBuffer);
		}
		if (scene->needUpdateSky)
		{
			switch (scene->skyType)
			{
				case SkyType::atmosphere_scattering:
				{
					{
						auto cb = begineOnceCommandBuffer();
						auto fb = getFramebuffer(envrImage.get(), renderPass_image16);

						cb->beginRenderPass(renderPass_image16, fb.get());
						cb->bindPipeline(scatteringPipeline);
						auto dir = scene->sunLight->getAxis()[2];
						cb->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(dir), &dir);
						cb->draw(3);
						cb->endRenderPass();

						endOnceCommandBuffer(cb);
					}

					// update IBL
					for (int i = 0; i < 3; i++)
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
					break;
				}
				case SkyType::panorama:
					// TODO : FIX SKY FROM FILE
					//if (skyImage)
					//{
					//	//writes.push_back(vk->writeDescriptorSet(panoramaPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, skyImage->getInfo(colorSampler), 0));
					//	//writes.push_back(vk->writeDescriptorSet(deferredPipeline.m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, radianceImage->getInfo(colorSampler), 0));

					//	AmbientBufferShaderStruct stru;
					//	stru.v = glm::vec4(1.f, 1.f, 1.f, skyImage->level - 1);
					//	stru.fogcolor = glm::vec4(0.f, 0.f, 1.f, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
					//	ambientBuffer->update(&stru, *stagingBuffer);
					//}
					break;
			}
		}
		if (scene->needUpdateAmbientBuffer)
		{
			AmbientBufferShaderStruct stru;
			stru.color = scene->ambientColor;
			stru.envr_max_mipmap = envrImage->levels.size() - 1;
			stru.fogcolor = glm::vec4(scene->fogColor, 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
			ambientBuffer->update(&stru, stagingBuffer);
		}
		if (scene->objects.size() > 0)
		{
			int updateCount = 0;
			std::vector<VkBufferCopy> staticUpdateRanges;
			std::vector<VkBufferCopy> animatedUpdateRanges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * scene->objects.size());
			int staticObjectIndex = 0;
			int animatedObjectIndex = 0;

			for (auto &o : scene->objects)
			{
				if (!o->model->animated)
				{
					if (o->changed)
					{
						auto srcOffset = sizeof(glm::mat4) * updateCount;
						memcpy(map + srcOffset, &o->getMat(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * staticObjectIndex;
						range.size = sizeof(glm::mat4);
						staticUpdateRanges.push_back(range);

						updateCount++;
					}
					o->sceneIndex = staticObjectIndex;
					staticObjectIndex++;
				}
				else
				{
					if (o->changed)
					{
						auto srcOffset = sizeof(glm::mat4) * updateCount;
						memcpy(map + srcOffset, &o->getMat(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * animatedObjectIndex;
						range.size = sizeof(glm::mat4);
						animatedUpdateRanges.push_back(range);

						updateCount++;
					}
					o->sceneIndex = animatedObjectIndex;
					animatedObjectIndex++;
				}

			}
			stagingBuffer->unmap();
			if (staticUpdateRanges.size() > 0)
				copyBuffer(stagingBuffer->v, staticObjectMatrixBuffer->v, staticUpdateRanges.size(), staticUpdateRanges.data());
			if (animatedUpdateRanges.size() > 0)
				copyBuffer(stagingBuffer->v, animatedObjectMatrixBuffer->v, animatedUpdateRanges.size(), animatedUpdateRanges.data());
		}

		std::vector<VkWriteDescriptorSet> writes;

		if (scene->terrain)
		{
			if (scene->terrain->changed)
			{
				TerrainShaderStruct stru;
				stru.coord = scene->terrain->getCoord();
				stru.blockCx = scene->terrain->block_cx;
				stru.blockSize = scene->terrain->block_size;
				stru.height = scene->terrain->height;
				stru.tessellationFactor = scene->terrain->tessellation_factor;
				stru.textureUvFactor = scene->terrain->texture_uv_factor;
				stru.mapDimension = scene->terrain->heightMap->levels[0].cx;

				terrainBuffer->update(&stru, stagingBuffer);

				if (scene->terrain->heightMap)
					writes.push_back(ds_terrain->imageWrite(TerrainHeightMapBinding, 0, scene->terrain->heightMap, colorBorderSampler));
				if (scene->terrain->normalMap)
					writes.push_back(ds_terrain->imageWrite(TerrainNormalMapBinding, 0, scene->terrain->normalMap, colorBorderSampler));
				if (scene->terrain->blendMap)
					writes.push_back(ds_terrain->imageWrite(TerrainBlendMapBinding, 0, scene->terrain->blendMap, colorBorderSampler));
				for (int i = 0; i < 4; i++)
				{
					if (scene->terrain->colorMaps[i])
						writes.push_back(ds_terrain->imageWrite(TerrainColorMapsBinding, i, scene->terrain->colorMaps[i], colorWrapSampler));
				}
				for (int i = 0; i < 4; i++)
				{
					if (scene->terrain->normalMaps[i])
						writes.push_back(ds_terrain->imageWrite(TerrainNormalMapsBinding, i, scene->terrain->normalMaps[i], colorWrapSampler));
				}
			}
		}
		if (scene->waters.size() > 0)
		{
			int updateCount = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(WaterShaderStruct) * scene->waters.size());

			for (auto &w : scene->waters)
			{
				if (w->changed)
				{
					auto offset = sizeof(WaterShaderStruct) * updateCount;
					WaterShaderStruct stru;
					stru.coord = w->getCoord();
					stru.blockCx = w->blockCx;
					stru.blockSize = w->blockSize;
					stru.height = w->height;
					stru.tessellationFactor = w->tessellationFactor;
					stru.textureUvFactor = w->textureUvFactor;
					stru.mapDimension = 1024;
					memcpy(map + offset, &stru, sizeof(WaterShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = offset;
					range.dstOffset = offset;
					range.size = sizeof(WaterShaderStruct);
					ranges.push_back(range);

					updateCount++;
				}
			}
			stagingBuffer->unmap();
			if (ranges.size() > 0) copyBuffer(stagingBuffer->v, waterBuffer->v, ranges.size(), ranges.data());
		}

		std::vector<Object*> staticObjects;
		std::vector<Object*> animatedObjects;
		int staticIndirectCount = 0;
		int animatedIndirectCount = 0;
		if (scene->needUpdateIndirectBuffer)
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

					if (!m->animated)
					{
						for (auto &g : m->geometries)
						{
							VkDrawIndexedIndirectCommand command = {};
							command.instanceCount = 1;
							command.indexCount = g->indiceCount;
							command.vertexOffset = m->vertexBase;
							command.firstIndex = m->indiceBase + g->indiceBase;
							command.firstInstance = (staticIndex << 8) + g->material->sceneIndex;

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
							command.firstInstance = (animatedIndex << 8) + g->material->sceneIndex;

							animatedCommands.push_back(command);
						}

						writes.push_back(ds_mrtAnim_bone->bufferWrite(0, animatedIndex, o->animationComponent->boneMatrixBuffer));

						animatedObjects.push_back(o.get());
						animatedIndex++;
					}
				}

				staticIndirectCount = staticCommands.size();
				animatedIndirectCount = animatedCommands.size();

				if (staticCommands.size() > 0)
					staticObjectIndirectBuffer->update(staticCommands.data(), stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * staticCommands.size());
				if (animatedCommands.size() > 0)
					animatedObjectIndirectBuffer->update(animatedCommands.data(), stagingBuffer, sizeof(VkDrawIndexedIndirectCommand) * animatedCommands.size());
			}
		}
		if (scene->needUpdateLightCount)
		{ // light count in light attribute
			auto count = scene->lights.size();
			lightBuffer->update(&count, stagingBuffer, 4);
		}

		std::vector<Light*> shadowLights;
		if (enable_shadow)
		{
			shadowLights.clear();
			if (scene->lights.size() > 0)
			{
				auto shadowIndex = 0;
				std::vector<VkBufferCopy> ranges;
				auto map = (unsigned char*)stagingBuffer->map(0, sizeof(glm::mat4) * scene->lights.size());

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

							writes.push_back(ds_defe->imageWrite(ShadowImageBinding, shadowIndex, esmImage.get(), colorSampler, 0, 1, shadowIndex, 1));
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
				stagingBuffer->unmap();
				if (ranges.size() > 0) copyBuffer(stagingBuffer->v, shadowBuffer->v, ranges.size(), ranges.data());
			}
		}
		if (scene->lights.size() > 0)
		{ // light attribute
			int lightIndex = 0;
			std::vector<VkBufferCopy> ranges;
			auto map = (unsigned char*)stagingBuffer->map(0, sizeof(LightShaderStruct) * scene->lights.size());
			for (auto &l : scene->lights)
			{
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
			stagingBuffer->unmap();
			if (ranges.size() > 0)
				copyBuffer(stagingBuffer->v, lightBuffer->v, ranges.size(), ranges.data());
		}

		updateDescriptorSets(writes.size(), writes.data());

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

				cb->bindVertexBuffer2(vertexStatBuffer, vertexAnimBuffer);
				cb->bindIndexBuffer(indexBuffer);

				// static
				if (staticObjects.size() > 0)
				{
					cb->bindPipeline(esmPipeline);
					VkDescriptorSet sets[] = {
						ds_esm->v,
						ds_textures->v
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
						ds_textures->v,
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

		cb->bindVertexBuffer2(vertexStatBuffer, vertexAnimBuffer);
		cb->bindIndexBuffer(indexBuffer);

		// mrt
		// static
		if (staticIndirectCount > 0)
		{
			cb->bindPipeline(mrtPipeline);
			VkDescriptorSet sets[] = {
				ds_mrt->v,
				ds_textures->v
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
				ds_textures->v,
				ds_mrtAnim_bone->v
			};
			cb->bindDescriptorSet(sets, 0, TK_ARRAYSIZE(sets));
			cb->drawIndirectIndex(animatedObjectIndirectBuffer.get(), animatedIndirectCount);
		}
		// terrain
		if (scene->terrain)
		{
			cb->bindPipeline(terrainPipeline);
			cb->bindDescriptorSet(&ds_terrain->v);
			cb->draw(4, 0, scene->terrain->block_cx * scene->terrain->block_cx);
		}
		// water
		if (scene->waters.size() > 0)
		{
			int index = 0;
			for (auto &w : scene->waters)
			{
				cb->bindPipeline(waterPipeline);
				cb->bindDescriptorSet(&ds_water->v);
				cb->draw(4, 0, w->blockCx * w->blockCx);
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
	}
}
