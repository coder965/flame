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
		if (cb_list)
			cb->waitEvents(1, &cb_list->last_event);

		do_render(framebuffer, clear, camera, count, user_data);

		if (cb_list) 
		{
			cb->resetEvent(cb_list->last_event);
			cb->setEvent(renderFinished);
		}
		cb->end();
		if (cb_list)
			cb_list->add(cb->v, renderFinished);
	}

	static Pipeline *pipeline_plain;
	static Pipeline *pipeline_plain_anim;
	static Pipeline *pipeline_frontlight;
	static Pipeline *pipeline_texture;
	static Pipeline *pipeline_texture_anim;
	bool PlainRenderer::first = true;
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

			first = false;
		}
	}

	void PlainRenderer::do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int _count, void *user_data)
	{
		auto mode = TK_HIGH(_count);
		auto count = TK_LOW(_count);

		cb->beginRenderPass(clear ? renderPass_depthC_image8C : renderPass_depthC_image8, framebuffer);
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
						updateDescriptorSets(1, &pipeline_plain_anim->descriptorSet->bufferWrite(0, 0, d.bone_buffer));
						cb->bindDescriptorSet();
					}
					break;
				case 1:
					cb->bindPipeline(pipeline_frontlight);
					break;
				case 2:
					cb->bindPipeline(!animated ? pipeline_texture : pipeline_texture_anim);
					break;
			}

			pc.modelview = camera->getMatInv() * d.mat;
			pc.color = d.color;
			cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
			cb->drawModel(model);
		}
	}

	static Pipeline *pipeline_wireframe;
	static Pipeline *pipeline_wireframe_anim;
	bool WireframeRenderer::first = true;
	WireframeRenderer::WireframeRenderer()
	{
		if (first)
		{
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

		ds_anim = std::make_unique<DescriptorSet>(pipeline_wireframe_anim);
	}

	void WireframeRenderer::do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data)
	{
		auto obj = (Object*)user_data;
		auto model = obj->model.get();
		auto animated = model->animated;

		cb->beginRenderPass(renderPass_image8, framebuffer);

		struct
		{
			glm::mat4 modelview;
			glm::mat4 proj;
			glm::vec4 color;
		}pc;
		pc.proj = matPerspective;

		{
			VkBuffer buffers[] = {
				vertexStatBuffer->v,
				vertexAnimBuffer->v
			};
			VkDeviceSize offsets[] = {
				0,
				0
			};
			cb->bindVertexBuffer(buffers, TK_ARRAYSIZE(buffers), offsets);
		}
		cb->bindIndexBuffer(indexBuffer);
		cb->bindPipeline(animated ? pipeline_wireframe_anim : pipeline_wireframe);
		if (animated)
		{
			if (last_obj != obj)
				updateDescriptorSets(1, &ds_anim->bufferWrite(0, 0, obj->animationComponent->boneMatrixBuffer));
			cb->bindDescriptorSet(&ds_anim->v);
		}
		pc.modelview = camera->getMatInv() * obj->getMat();
		pc.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
		cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
		cb->drawModel(model);

		cb->endRenderPass();
		last_obj = obj;
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
	static Pipeline *mrtPipeline;
	static Pipeline *mrtAnimPipeline;
	static Pipeline *terrainPipeline;
	static Pipeline *waterPipeline;
	static Pipeline *deferredPipeline;
	static Pipeline *composePipeline;
	static Pipeline *esmPipeline;
	static Pipeline *esmAnimPipeline;
	static RenderPass *defeRenderPass;
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

	void DeferredRenderer::update(Scene *scene)
	{
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
	}

	void DeferredRenderer::do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data)
	{

	}
}
