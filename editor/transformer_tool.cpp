#include "..\src\core.h"
#include "..\src\model.h"

#include "transformer_tool.h"

TransformerTool::TransformerTool(tke::Framebuffer *_fb)
	:Tool(_fb)
{
}

void TransformerTool::show(glm::mat4 &matProj, VkEvent waitEvent)
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

			cb->beginRenderPass(tke::plainRenderPass_image8, fb);

			cb->bindVertexBuffer(tke::staticVertexBuffer);
			cb->bindIndexBuffer(tke::staticIndexBuffer);
			cb->bindPipeline(tke::plain3dPipeline);
			cb->bindDescriptorSet();
			struct
			{
				glm::mat4 matrix;
				glm::vec4 color;
			}data;
			data.matrix = matProj * glm::translate(0.f, 0.f, -5.f);
			data.color = glm::vec4(1.f);
			cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, 80, &data);
			cb->drawIndex(tke::arrowModel->indices.size(), tke::arrowModel->indiceBase, tke::arrowModel->vertexBase);

			cb->endRenderPass();
			break;
		}
	}

	cb->resetEvent(waitEvent);
	cb->setEvent(renderFinished);

	cb->end();
}
