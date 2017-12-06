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

	void WireframeRenderer::render(Framebuffer *framebuffer, Camera *camera, FrameCommandBufferList *cb_list, void *user_data)
	{
		cb->reset();
		cb->begin();

		cb->waitEvents(1, &cb_list->last_event);

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
		pc.proj = tke::matPerspective;

		{
			VkBuffer buffers[] = {
				tke::vertexStatBuffer->v,
				tke::vertexAnimBuffer->v
			};
			VkDeviceSize offsets[] = {
				0,
				0
			};
			cb->bindVertexBuffer(buffers, TK_ARRAYSIZE(buffers), offsets);
		}
		cb->bindIndexBuffer(tke::indexBuffer);
		cb->bindPipeline(animated ? tke::pipeline_wireframe_anim : tke::pipeline_wireframe);
		if (animated)
		{
			if (last_obj != obj)
				tke::updateDescriptorSets(1, &ds_anim->bufferWrite(0, 0, obj->animationComponent->boneMatrixBuffer));
			cb->bindDescriptorSet(&ds_anim->v);
		}
		pc.modelview = camera->getMatInv() * obj->getMat();
		pc.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
		cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
		cb->drawModel(model);

		cb->endRenderPass();
		last_obj = obj;

		cb->resetEvent(cb_list->last_event);
		cb->setEvent(renderFinished);

		cb->end();

		cb_list->add(cb->v, renderFinished);
	}

	DeferredRenderer::DeferredRenderer()
	{

	}

	void DeferredRenderer::render(Framebuffer *framebuffer, Camera *camera, FrameCommandBufferList *cb_list, void *user_data)
	{

	}
}
