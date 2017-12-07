#include "renderer.h"
#include "synchronization.h"

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

	DeferredRenderer::DeferredRenderer()
	{

	}

	void DeferredRenderer::do_render(Framebuffer *framebuffer, bool clear, Camera *camera, int count, void *user_data)
	{

	}
}
