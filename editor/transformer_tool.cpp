#include "../src/core.h"

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
	if (!currentTransformerTool->transformer || currentTransformerTool->mode == TransformerTool::ModeNull)
		return;

	auto dir = currentTransformerTool->transformer->getCoord() - currentCamera->getCoord();
	if (glm::length(dir) <= 0.f)
		return;
	dir = glm::normalize(dir);
	auto coord = currentCamera->getCoord() + dir * 5.f;

	tke::Model *model = currentTransformerTool->mode == TransformerTool::ModeMove ? tke::arrowModel : 
		(currentTransformerTool->mode == TransformerTool::ModeRotate ? tke::torusModel : tke::hamerModel);

	cb->bindVertexBuffer(tke::staticVertexBuffer);
	cb->bindIndexBuffer(tke::staticIndexBuffer);
	cb->bindPipeline(currentDrawPolicy == 0 ? tke::pipeline_headlight : tke::pipeline_plain);
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
	cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(data), &data);
	cb->drawIndex(model->indices.size(), model->indiceBase, model->vertexBase);

	data.modelview = currentCamera->getMatInv() * glm::translate(coord) * glm::rotate(90.f, glm::vec3(0, 0, 1));
	data.color = currentDrawPolicy == 0 ? (currentTransformerTool->selectedAxis == 1 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 1.f, 0.f, 1.f)) : glm::vec4(2.f / 255.f, 0.f, 0.f, 0.f);
	cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(data), &data);
	cb->drawIndex(model->indices.size(), model->indiceBase, model->vertexBase);

	data.modelview = currentCamera->getMatInv() * glm::translate(coord) * glm::rotate(-90.f, glm::vec3(0, 1, 0));
	data.color = currentDrawPolicy == 0 ? (currentTransformerTool->selectedAxis == 2 ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f)) : glm::vec4(3.f / 255.f, 0.f, 0.f, 0.f);
	cb->pushConstant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(data), &data);
	cb->drawIndex(model->indices.size(), model->indiceBase, model->vertexBase);
}

bool TransformerTool::leftDown(int x, int y)
{
	if (!transformer)
		return false;
	currentDrawPolicy = 1;
	auto index = tke::pickUp(x, y, draw);
	selectedAxis = index - 1;
	return index != 0;
}

void TransformerTool::mouseMove(int _xDisp, int _yDisp)
{
	if (!currentTransformerTool->transformer || currentTransformerTool->mode == TransformerTool::ModeNull || selectedAxis == -1)
		return;

	switch (mode)
	{
	case ModeMove:
	{
		auto xDisp = (float)_xDisp / (float)tke::resCx;
		auto yDisp = (float)_yDisp / (float)tke::resCy;

		auto p = tke::matPerspective * currentCamera->getMatInv() * glm::vec4(transformer->getCoord(), 1.f);
		p = p / p.w;
		p.x += xDisp;
		p.y += yDisp;
		p = tke::matPerspectiveInv * p;
		p = p / p.w;
		p = currentCamera->getMat() * p;

		switch (selectedAxis)
		{
		case 0:
			transformer->setCoordX(p.x);
			break;
		case 1:
			transformer->setCoordY(p.y);
			break;
		case 2:
			transformer->setCoordZ(p.z);
			break;
		}
	}
		break;
	}
}

void TransformerTool::show(tke::Camera *camera, VkEvent waitEvent, VkEvent signalEvent)
{
	cb->reset();
	cb->begin();

	cb->waitEvents(1, &waitEvent);

	cb->beginRenderPass(tke::renderPass_depthC_image8, fb);

	currentTransformerTool = this;
	currentCamera = camera;
	currentDrawPolicy = 0;
	draw(cb);

	cb->endRenderPass();

	cb->resetEvent(waitEvent);
	cb->setEvent(signalEvent);

	cb->end();
}
