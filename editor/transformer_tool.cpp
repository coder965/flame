#include "..\src\core.h"
#include "..\src\model.h"

#include "transformer_tool.h"

TransformerTool::TransformerTool(tke::Framebuffer *_fb)
	:Tool(_fb)
{
}

static TransformerTool *currentTransformerTool = nullptr;
static tke::Camera *currentCamera = nullptr;
static int currentDrawPolicy = 0;
static void draw(tke::CommandBuffer *cb)
{
	if (currentTransformerTool->transformer)
	{
		switch (currentTransformerTool->mode)
		{
		case TransformerTool::ModeNull:
			break;
		case TransformerTool::ModeMove:
		{
			auto dir = currentTransformerTool->transformer->getCoord() - currentCamera->getCoord();
			if (glm::length(dir) > 0.f)
			{
				dir = glm::normalize(dir);

				auto coord = currentCamera->getCoord() + dir * 5.f;

				cb->bindVertexBuffer(tke::staticVertexBuffer);
				cb->bindIndexBuffer(tke::staticIndexBuffer);
				cb->bindPipeline(currentDrawPolicy == 0 ? tke::plainPipeline_3d_normal_depth : tke::plainPipeline_3d_depth);
				cb->bindDescriptorSet();

				struct
				{
					glm::mat4 modelview;
					glm::mat4 proj;
					glm::vec4 color;
				}data;

				data.proj = tke::matPerspective;

				data.modelview = currentCamera->getMatInv() * glm::translate(coord);
				data.color = currentDrawPolicy == 0 ? (currentTransformerTool->selectedAxis == 0 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(1.f, 0.f, 0.f, 1.f)) : glm::vec4(1.f / 255.f, 0.f, 0.f, 0.f);
				cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
				cb->drawIndex(tke::arrowModel->indices.size(), tke::arrowModel->indiceBase, tke::arrowModel->vertexBase);

				data.modelview = currentCamera->getMatInv() * glm::translate(coord) * glm::rotate(90.f, glm::vec3(0, 0, 1));
				data.color = currentDrawPolicy == 0 ? (currentTransformerTool->selectedAxis == 1 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 1.f, 0.f, 1.f)) : glm::vec4(2.f / 255.f, 0.f, 0.f, 0.f);
				cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
				cb->drawIndex(tke::arrowModel->indices.size(), tke::arrowModel->indiceBase, tke::arrowModel->vertexBase);

				data.modelview = currentCamera->getMatInv() * glm::translate(coord) * glm::rotate(-90.f, glm::vec3(0, 1, 0));
				data.color = currentDrawPolicy == 0 ? (currentTransformerTool->selectedAxis == 2 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f)) : glm::vec4(3.f / 255.f, 0.f, 0.f, 0.f);
				cb->pushConstant(tke::StageType((int)tke::StageType::vert | (int)tke::StageType::frag), 0, sizeof(data), &data);
				cb->drawIndex(tke::arrowModel->indices.size(), tke::arrowModel->indiceBase, tke::arrowModel->vertexBase);
			}
		}
			break;
		case TransformerTool::ModeRotate:
			break;
		case TransformerTool::ModeScale:
			break;
		}
	}
}

bool TransformerTool::leftDown(int x, int y)
{
	currentDrawPolicy = 1;
	auto index = tke::pickUp(x, y, draw);
	selectedAxis = index - 1;
	return index != 0;
}

void TransformerTool::show(tke::Camera *camera, VkEvent waitEvent)
{
	cb->reset();
	cb->begin();

	cb->waitEvents(1, &waitEvent);

	cb->beginRenderPass(tke::plainRenderPass_depth_clear_image8, fb);

	currentTransformerTool = this;
	currentCamera = camera;
	currentDrawPolicy = 0;
	draw(cb);

	cb->endRenderPass();

	cb->resetEvent(waitEvent);
	cb->setEvent(renderFinished);

	cb->end();
}
