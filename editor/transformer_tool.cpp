#include "..\src\core.h"
#include "..\src\model.h"

#include "transformer_tool.h"

TransformerTool::TransformerTool(tke::Framebuffer *_fb)
	:Tool(_fb)
{
}

void TransformerTool::show(const glm::mat4 &matView, VkEvent waitEvent)
{
	cb->reset();
	cb->begin();

	cb->waitEvents(1, &waitEvent);

	if (transformer)
	{
		switch (mode)
		{
		case ModeNull:
			break;
		case ModeMove:
		{
			cb->beginRenderPass(tke::plainRenderPass_depth_clear_image8, fb);

			cb->bindVertexBuffer(tke::staticVertexBuffer);
			cb->bindIndexBuffer(tke::staticIndexBuffer);
			cb->bindPipeline(tke::plainPipeline_3d_normal_depth);
			cb->bindDescriptorSet();

			auto rotMat3 = glm::mat3(matView);
			auto rotMat = glm::mat4(rotMat3);

			struct
			{
				glm::mat4 modelview;
				glm::mat4 proj;
				glm::vec4 color;
			}data;

			data.modelview = glm::translate(0.f, 0.f, -5.f) * rotMat;
			data.proj = tke::matPerspective;
			data.color = glm::vec4(1.f, 0.f, 0.f, 1.f);
			cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
			cb->drawIndex(tke::arrowModel->indices.size(), tke::arrowModel->indiceBase, tke::arrowModel->vertexBase);

			data.modelview = glm::translate(0.f, 0.f, -5.f) * rotMat * glm::rotate(90.f, glm::vec3(0, 0, 1));
			data.proj = tke::matPerspective;
			data.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
			cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
			cb->drawIndex(tke::arrowModel->indices.size(), tke::arrowModel->indiceBase, tke::arrowModel->vertexBase);

			data.modelview = glm::translate(0.f, 0.f, -5.f) * rotMat * glm::rotate(-90.f, glm::vec3(0, 1, 0));
			data.proj = tke::matPerspective;
			data.color = glm::vec4(0.f, 0.f, 1.f, 1.f);
			cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
			cb->drawIndex(tke::arrowModel->indices.size(), tke::arrowModel->indiceBase, tke::arrowModel->vertexBase);

			cb->endRenderPass();
		}
			break;
		case ModeRotate:
			break;
		case ModeScale:
			break;
		}
	}

	cb->resetEvent(waitEvent);
	cb->setEvent(renderFinished);

	cb->end();
}
