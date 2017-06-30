#include "..\src\core.h"

#include "transformer_tool.h"

TransformerTool::TransformerTool(tke::Framebuffer *_fb)
	:Tool(_fb)
{
}

void TransformerTool::show(VkEvent waitEvent)
{
	cb->reset();
	cb->begin();

	cb->waitEvents(1, &waitEvent);

	cb->beginRenderPass(tke::plainRenderPass_image8, fb);

	cb->bindVertexBuffer(tke::staticVertexBuffer);
	cb->bindIndexBuffer(tke::staticIndexBuffer);


	cb->endRenderPass();

	cb->resetEvent(waitEvent);
	cb->setEvent(event);

	cb->end();
}
